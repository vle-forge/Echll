/*
 * Copyright (C) 2013-2014 INRA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __VLE_KERNEL_DSDE_HPP__
#define __VLE_KERNEL_DSDE_HPP__

#include <vle/time.hpp>
#include <vle/port.hpp>
#include <vle/heap.hpp>
#include <memory>
#include <set>
#include <vle/dbg.hpp>

namespace vle {

struct dsde_internal_error : std::runtime_error
{
    explicit dsde_internal_error(const std::string& msg)
        : std::runtime_error(msg)
    {}
};

namespace dsde
{
    template <typename Time, typename Value>
    struct Model
    {
        typedef typename Time::type time_type;
        typedef Value value_type;

        Model()
            : tl(-Time::infinity), tn(Time::infinity), parent(nullptr)
        {}

        Model(std::initializer_list <std::string> lst_x,
              std::initializer_list <std::string> lst_y)
            : x(lst_x), y(lst_y), tl(-Time::infinity), tn(Time::infinity),
            parent(nullptr)
        {}

        virtual ~Model()
        {}

        mutable vle::PortList <Value> x, y;
        time_type tl, tn;
        Model *parent;
        typename HeapType <Time, Value>::handle_type heapid;

        virtual void start(const time_type& time) = 0;
        virtual void transition(const time_type& time) = 0;
        virtual void output(const time_type& time) = 0;
    };

    template <typename Time, typename Value>
    using Bag = std::set <Model <Time, Value>*>;

    template <typename Time, typename Value>
    using UpdatedPort = std::set <const Model <Time, Value>*>;

    template <typename Time, typename Value>
    struct AtomicModel : Model <Time, Value>
    {
        typedef typename Time::type time_type;
        typedef Value value_type;

        virtual time_type init(const time_type& time) = 0;
        virtual time_type delta(const time_type& time) = 0;
        virtual void lambda() const = 0;

        AtomicModel()
            : Model <Time, Value>()
        {}

        AtomicModel(std::initializer_list <std::string> lst_x,
                    std::initializer_list <std::string> lst_y)
            : Model <Time, Value>(lst_x, lst_y)
        {}

        virtual ~AtomicModel()
        {}

        virtual void start(const time_type& time) override final
        {
            Model <Time, Value>::tl = time;
            Model <Time, Value>::tn = time + init(time);
        }

        virtual void transition(const time_type& time) override final
        {
#ifndef VLE_OPTIMIZE
            if (!(Model <Time, Value>::tl <= time && time <= Model <Time, Value>::tn))
                throw dsde_internal_error("Synchronization error");

            if (time < Model <Time, Value>::tn and Model <Time, Value>::x.is_empty())
                return;
#endif
            Model <Time, Value>::tn = time + delta(time - Model <Time, Value>::tl);
            Model <Time, Value>::tl = time;
            Model <Time, Value>::x.clear();
        }

        virtual void output(const time_type& time) override final
        {
            if (time == Model <Time, Value>::tn)
                lambda();
        }
    };

    template <typename Time, typename Value>
    struct CoupledModel : Model <Time, Value>
    {
        typedef typename Time::type time_type;
        typedef Value value_type;

        HeapType <Time, Value> heap;
        UpdatedPort <Time, Value> last_output_list;

        typedef std::vector <Model <Time, Value>*> children_t;

        /**
         * @brief Get the children of the @e CoupledModel.
         *
         * The @e children function is called only once by the simulation layer
         * after the constructor.
         *
         * @return
         */
        virtual children_t children() = 0;

        virtual void post(const UpdatedPort <Time, Value> &out,
                          UpdatedPort <Time, Value> &in) const = 0;

        CoupledModel()
            : Model <Time, Value>()
        {}

        CoupledModel(std::initializer_list <std::string> lst_x,
                     std::initializer_list <std::string> lst_y)
            : Model <Time, Value>(lst_x, lst_y)
        {}

        virtual ~CoupledModel()
        {}

        virtual void start(const time_type& time) override final
        {
            auto cs = children();
            std::for_each(cs.begin(), cs.end(),
                          [=](Model <Time, Value>* child)
                          {
                              child->parent = this;
                              child->start(time);

                              auto id = heap.emplace(child, child->tn);
                              child->heapid = id;
                              (*id).heapid = id;
                          });

            Model <Time, Value>::tl = time;
            Model <Time, Value>::tn = heap.top().tn;
        }

        virtual void transition(const time_type& time) override final
        {
#ifndef VLE_OPTIMIZE
            if (!(Model <Time, Value>::tl <= time && time <= Model <Time, Value>::tn))
                throw dsde_internal_error("Synchronization error");

            if (time < Model <Time, Value>::tn && Model <Time, Value>::x.is_empty())
                return;
#endif
            Bag <Time, Value> bag;

            {
                auto it = heap.ordered_begin();
                auto et = heap.ordered_end();

                for (; it != et && (*it).tn == time; ++it)
                    bag.insert(
                        reinterpret_cast <Model <Time, Value>*>(
                            (*it).element));
            }

            {
                if (not Model <Time, Value>::x.is_empty())
                    post({this}, last_output_list);

                for (auto &child : last_output_list)
                    bag.insert(const_cast <Model <Time, Value>*>(child));

                last_output_list.clear();
            }

            std::for_each(bag.begin(), bag.end(),
                          [=](Model <Time, Value> *child)
                          {
                              child->transition(time);
                              (*child->heapid).tn = child->tn;
                              heap.update(child->heapid);
                          });

            Model <Time, Value>::tl = time;
            Model <Time, Value>::tn = heap.top().tn;
            Model <Time, Value>::x.clear();
        }

        virtual void output(const time_type& time) override final
        {
#ifndef VLE_OPTIMIZE
            if (!(time == heap.top().tn))
                throw dsde_internal_error("Synchronization error");

            if (!(Model <Time, Value>::tn == heap.top().tn))
                throw dsde_internal_error("Synchronization error");
#endif
            if (time == Model <Time, Value>::tn && not heap.empty()) {
                UpdatedPort <Time, Value> lst;
                auto it = heap.ordered_begin();
                auto et = heap.ordered_end();

                do {
                    auto id = (*it).heapid;
                    Model <Time, Value> *mdl =
                        reinterpret_cast <Model <Time, Value>*>(
                            (*id).element);

                    mdl->output(time);
                    if (!(mdl->y.is_empty()))
                        lst.emplace(mdl);
                    ++it;
                } while (it != et && it->tn == Model <Time, Value>::tn);

                post(lst, last_output_list);

                std::for_each(lst.begin(), lst.end(),
                              [](const Model <Time, Value> *child)
                              {
                                  child->y.clear();
                              });
            }
        }
    };

    template <typename Time, typename Value>
    struct Executive : Model <Time, Value>
    {
        typedef typename Time::type time_type;
        typedef Value value_type;

        HeapType <Time, Value> heap;
        UpdatedPort <Time, Value> last_output_list;

        typedef std::vector <Model <Time, Value>*> children_t;
        time_type chi_tl, chi_tn;
        typename HeapType <Time, Value>::handle_type chi_heapid;
        mutable vle::PortList <Value> chi_x, chi_y;

        virtual children_t children() = 0;
        virtual time_type init(const time_type& time) = 0;
        virtual time_type delta(const time_type& time) = 0;
        virtual void lambda() const = 0;
        virtual void post(const UpdatedPort <Time, Value> &y,
                          UpdatedPort <Time, Value> &x) const = 0;

        Executive()
            : Model <Time, Value>()
        {}

        Executive(std::initializer_list <std::string> lst_x,
                  std::initializer_list <std::string> lst_y)
            : Model <Time, Value>(lst_x, lst_y)
        {}

        Executive(std::initializer_list <std::string> lst_x,
                  std::initializer_list <std::string> lst_y,
                  std::initializer_list <std::string> chi_lst_x,
                  std::initializer_list <std::string> chi_lst_y)
            : Model <Time, Value>(lst_x, lst_y), chi_tl(-Time::infinity),
            chi_tn(Time::infinity), chi_x(chi_lst_x), chi_y(chi_lst_y)
        {}

        virtual ~Executive()
        {}

        void insert(Model <Time, Value> *mdl)
        {
            dWarning("Executive insert a new model");
            mdl->parent = this;
            mdl->start(Executive::chi_tl);

            auto id = heap.emplace(mdl, mdl->tn);
            mdl->heapid = id;
            (*id).heapid = id;
        }

        void erase(Model <Time, Value >*mdl)
        {
            dWarning("Execute erase a model");
            last_output_list.erase(mdl);
            heap.erase(mdl->heapid);
            mdl->parent = nullptr;
        }

        virtual void start(const time_type& time) override final
        {
            chi_tl = time;
            chi_tn = time + init(time);
            auto id = heap.emplace(this, chi_tn);
            chi_heapid = id;
            (*id).heapid = id;

            auto cs = children();
            std::for_each(cs.begin(), cs.end(),
                          [=](Model <Time, Value> *child)
                          {
                              child->parent = this;
                              child->start(time);

                              auto id = heap.emplace(child, child->tn);
                              (*id).heapid = id;
                              child->heapid = id;
                          });

            Model <Time, Value>::tl = time;
            Model <Time, Value>::tn = heap.top().tn;
            Model <Time, Value>::x.clear();
        }

        virtual void transition(const time_type &time) override final
        {
#ifndef VLE_OPTIMIZE
            if (!(Model <Time, Value>::tl <= time && time <= Model <Time, Value>::tn))
                throw dsde_internal_error("Synchronization error");

            if (time < Model <Time, Value>::tn && Model <Time, Value>::x.is_empty())
                return;
#endif
            Bag <Time, Value> bag;
            bool have_chi = false;

            {
                auto it = heap.ordered_begin();
                auto et = heap.ordered_end();

                for (; it != et && (*it).tn == time; ++it) {
                    if ((*it).element == this)
                        have_chi = true;
                    else
                        bag.insert(
                            reinterpret_cast <Model <Time, Value>*>(
                                (*it).element));
                }
            }

            {
                if (not Model <Time, Value>::x.is_empty())
                    post({this}, last_output_list);

                for (auto &child : last_output_list) {
                    if (child == this)
                        have_chi = true;
                    else
                        bag.insert(const_cast <Model <Time, Value>*>(child));
                }

                last_output_list.clear();
            }

            std::for_each(bag.begin(), bag.end(),
                          [=](Model <Time, Value> *child)
                          {
                              child->transition(time);
                              (*child->heapid).tn = child->tn;
                              heap.update(child->heapid);
                          });

            if (have_chi) {
                time_type e = time - chi_tl;
                chi_tl = time;
                chi_tn = time + delta(e);
                (*chi_heapid).tn = chi_tn;
                heap.update(chi_heapid);
            }

            Model <Time, Value>::tn = heap.top().tn;
            Model <Time, Value>::tl = time;
            Model <Time, Value>::x.clear();
        }

        virtual void output(const time_type &time) override final
        {
#ifndef VLE_OPTIMIZE
            if (!(time == heap.top().tn))
                throw dsde_internal_error("Synchronization error");

            if (!(Model <Time, Value>::tn == heap.top().tn))
                throw dsde_internal_error("Synchronization error");
#endif
            if (time == Model <Time, Value>::tn && not heap.empty()) {
                UpdatedPort <Time, Value> lst;
                auto it = heap.ordered_begin();
                auto et = heap.ordered_end();

                do {
                    auto id = (*it).heapid;
                    Model <Time, Value> *mdl =
                        reinterpret_cast <Model <Time, Value>*>((*id).element);

                    if (mdl == this) {
                        lambda();
                    } else {
                        mdl->output(time);
                    }
                    if (!(mdl->y.is_empty()))
                        lst.emplace(mdl);
                    ++it;
                } while (it != et && it->tn == Model <Time, Value>::tn);

                post(lst, last_output_list);

                std::for_each(lst.begin(), lst.end(),
                              [](const Model <Time, Value> *child)
                              {
                                child->y.clear();
                              });
            }
        }
    };

    template <typename Time, typename Value>
    struct Engine
    {
        typedef typename Time::type time_type;
        typedef Value value_type;
        typedef Model <Time, Value> model_type;

        time_type pre(model_type& model, const time_type& time)
        {
            model.start(time);

            return model.tn;
        }

        time_type run(model_type& model, const time_type& time)
        {
            model.output(time);
            model.transition(time);

            return model.tn;
        }

        void post(model_type& model, const time_type& time)
        {
            (void)model;
            (void)time;
        }
    };
};

};

#endif
