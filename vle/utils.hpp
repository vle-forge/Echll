/*
 * Copyright (C) 2014 INRA
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

#ifndef __VLE_KERNEL_UTILS_HPP__
#define __VLE_KERNEL_UTILS_HPP__

#include <functional>
#include <limits>
#include <cmath>

namespace vle
{
    /**
     * The @e ScopeExit structure permits the use of the @c Scope_Guard C++
     * idiom.
     *
     * @code
     * void world::add_person(person const& a_person)
     * {
     *     bool commit = false;
     *
     *     persons_.push_back(a_person);
     *     ScopeExit on_exit1([&commit, this](void)
     *     {
     *         if (!commit)
     *             persons_.pop_back();
     *     });
     *
     *     // ...
     *
     *     commit = true;
     * }
     * @endcode
     */
    struct ScopeExit
    {
        ScopeExit(std::function <void (void)> fct)
            : fct(fct)
        {
        }

        ~ScopeExit()
        {
            fct();
        }

    private:
        std::function<void (void)> fct;
    };

    template <typename T>
    bool is_almost_equal(const T a, const T b)
    {
        const T scale = (std::abs(a) + std::abs(b)) / T(2.0);
        return std::abs(a - b) <= (scale * std::numeric_limits<T>::epsilon());
    }
}

#endif