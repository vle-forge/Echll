/*
 * Copyright (C) 2015  INRA
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

#ifndef ORG_VLEPROJECT_KERNEL_DSDE_DSDE_DEBUG_HPP
#define ORG_VLEPROJECT_KERNEL_DSDE_DSDE_DEBUG_HPP

#include <vle/dsde/dsde.hpp>
#include <vle/debug.hpp>
#include <ostream>

namespace vle { namespace dsde {

template <typename Time, typename InputPort, typename OutputPort>
std::ostream& operator<<(std::ostream& out,
    const AtomicModel <Time, InputPort, OutputPort>& model)
{
    out << "AtomicModel " << &model
        << ": X=" << model.x
        << ", Y=" << model.y
        << ", tl=" << model.tl
        << ", tn=" << model.tn
        << ", parent=" << model.parent;

    return out;
}

// template <typename Time, typename InputPort, typename OutputPort>
// std::ostream& operator<<(std::ostream& out,
//     const UpdatedPort <Time, InputPort, OutputPort>& lst)
// {
//     if (not lst.empty()) {
//         out << '(';

//         auto it = lst.cbegin();
//         do {
//             out << *it++;
//         } while (it != lst.cend());

//         out << ')';
//     }

//     return out;
// }

template <typename Time, typename InputPort, typename OutputPort,
          typename ChildInputPort, typename ChildOutputPort>
std::ostream& operator<<(std::ostream& out,
    const CoupledModel <Time, InputPort, OutputPort, ChildInputPort,
                        ChildOutputPort>& model)
{
    using model_type = Model <Time, InputPort, OutputPort>;
    using atomicmodel_type = AtomicModel <Time, InputPort, OutputPort>;
    using coupledmodel_type = CoupledModel <Time, InputPort, OutputPort,
                                            ChildInputPort, ChildOutputPort>;
    using executive_type = Executive <Time, InputPort, OutputPort,
                                      ChildInputPort, ChildOutputPort>;

    out << "CoupledModel " << &model
        << ": X=" << model.x
        << ", Y=" << model.y
        << ", tl=" << model.tl
        << ", tn=" << model.tn
        << ", parent=" << model.parent
        // << ", last_output_list=" << model.last_output_list
        << ", child=\n";

    auto copyheap = model.heap;
    auto it = copyheap.ordered_begin();
    auto et = copyheap.ordered_end();

    for (; it != et; ++it) {
        const auto *mdl = reinterpret_cast <const model_type*>((*it).element);

        out << "- node " << it->element
            << " heapid " << &it->heapid << " tn " << it->tn;

        const auto *atom = dynamic_cast <const atomicmodel_type*>(mdl);
        if (atom) {
            out << '\t' << *atom;
        } else {
            const auto *cpld = dynamic_cast <const coupledmodel_type*>(mdl);
            if (cpld) {
                out << '\t' << *cpld;
            } else {
                out << '\t' << dynamic_cast <const executive_type*>(mdl);
            }
        }

        out << '\n';
    }

    return out;
}

template <typename Time, typename InputPort, typename OutputPort,
          typename ChildInputPort, typename ChildOutputPort>
std::ostream& operator<<(std::ostream& out,
    const Executive <Time, InputPort, OutputPort, ChildInputPort,
                     ChildOutputPort>& model)
{
    using model_type = Model <Time, InputPort, OutputPort>;
    using atomicmodel_type = AtomicModel <Time, InputPort, OutputPort>;
    using coupledmodel_type = CoupledModel <Time, InputPort, OutputPort,
                                            ChildInputPort, ChildOutputPort>;
    using executive_type = Executive <Time, InputPort, OutputPort,
                                      ChildInputPort, ChildOutputPort>;


    out << "Executive " << &model
        << ": chi_X=" << model.chi_x
        << ", chi_Y=" << model.chi_y
        << ", chi_tl=" << model.chi_tl
        << ", chi_tn=" << model.chi_tn
        << ": X=" << model.x
        << ", Y=" << model.y
        << ", tl=" << model.tl
        << ", tn=" << model.tn
        << ", e=" << model.e
        << ", parent=" << model.parent
        // << ", last_output_list=" << model.last_output_list
        << ", child=\n";

    auto copyheap = model.heap;
    auto it = copyheap.ordered_begin();
    auto et = copyheap.ordered_end();

    for (; it != et; ++it) {
        const auto *mdl = reinterpret_cast <const model_type*>((*it).element);

        out << "- node " << it->element
            << " heapid " << &it->heapid << " tn " << it->tn;

        const auto *atom = dynamic_cast <const atomicmodel_type*>(mdl);
        if (atom) {
            out << '\t' << *atom;
        } else {
            const auto *cpld = dynamic_cast <const coupledmodel_type*>(mdl);
            if (cpld) {
                out << '\t' << *cpld;
            } else {
                out << '\t' << dynamic_cast <const executive_type*>(mdl);
            }
        }

        out << '\n';
    }

    return out;
}

}}

#endif
