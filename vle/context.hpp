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

#ifndef ORG_VLEPROJECT_KERNEL_CONTEXT_HPP
#define ORG_VLEPROJECT_KERNEL_CONTEXT_HPP

#include <boost/any.hpp>
#include <memory>

namespace vle {

class ContextImpl;

using Context = std::shared_ptr <ContextImpl>;

/**
 * Default @e ContextImpl initializes logger system with the standard error
 * output (@e stderr).
 */
class ContextImpl
{
public:
    ContextImpl();

    ContextImpl(const std::string& filename);

    ContextImpl(const ContextImpl&) = default;

    ContextImpl& operator=(const ContextImpl&) = default;

    ContextImpl(ContextImpl&&) = default;

    ContextImpl& operator=(ContextImpl&&) = default;

    ~ContextImpl() = default;

    std::ostream& log() const;

    std::ostream& dbg() const;

    int get_log_priority() const;

    void set_log_priority(int priority);

    unsigned get_thread_number() const;

    void set_thread_number(unsigned thread_number);

    void set_user_data(const boost::any &user_data);

    bool is_on_tty() const;

    const boost::any& get_user_data() const;

    boost::any& get_user_data();

private:
    std::shared_ptr <std::ostream> m_log;
    boost::any   m_user_data;
    unsigned     m_thread_number = 1;
    int          m_log_priority  = 1;
    bool         m_is_a_tty;
};

}

#include <vle/detail/context-implementation.hpp>

#endif
