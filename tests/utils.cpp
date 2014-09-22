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

#include <vle/environment.hpp>
#include <vle/dbg.hpp>
#include <vle/path.hpp>
#include <vle/utils.hpp>
#include <cstring>

const std::string str500("01234567890123456789012345678901234567890123456789"
                         "01234567890123456789012345678901234567890123456789"
                         "01234567890123456789012345678901234567890123456789"
                         "01234567890123456789012345678901234567890123456789"
                         "01234567890123456789012345678901234567890123456789"
                         "01234567890123456789012345678901234567890123456789"
                         "01234567890123456789012345678901234567890123456789"
                         "01234567890123456789012345678901234567890123456789"
                         "01234567890123456789012345678901234567890123456789"
                         "01234567890123456789012345678901234567890123456789");

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("try-make_path_api", "run")
{
    {
        std::string path = vle::Path::make_path("A", "B", "C");
        REQUIRE(path == "A/B/C");
    }

    {
        std::string path = vle::Path::make_path("struct");
        REQUIRE(path == "struct");
    }
}

TEST_CASE("try-environment-package-path", "run")
{
    vle::EnvironmentPtr env = std::make_shared<vle::Environment>();

    std::string tmp_path = vle::Path::get_temporary_path();
    env->set_prefix_path(tmp_path);

    REQUIRE(env->get_prefix_path() == tmp_path);
    REQUIRE(env->get_package_path("test", vle::PACKAGE_DATA_DIRECTORY) ==
            vle::Path::make_path(tmp_path, "pkgs-2.0", "test", "data"));
    REQUIRE(env->get_package_path("test", vle::PACKAGE_EXP_DIRECTORY) ==
            vle::Path::make_path(tmp_path, "pkgs-2.0", "test", "exp"));
    REQUIRE(env->get_package_path("test", vle::PACKAGE_SIMULATOR_DIRECTORY) ==
            vle::Path::make_path(tmp_path, "pkgs-2.0", "test", "simulators"));
}

TEST_CASE("try-stringf-format", "run")
{
    std::string small = vle::stringf("%d %d %d", 1, 2, 3);
    REQUIRE(std::strcmp(small.c_str(), "1 2 3") == 0);

    REQUIRE(str500.size() == 500u);

    std::string big;

    REQUIRE_NOTHROW(big = vle::stringf("%s%s%s%s%s",
                                       str500.c_str(),
                                       str500.c_str(),
                                       str500.c_str(),
                                       str500.c_str(),
                                       str500.c_str()));

    REQUIRE(big.size() == (500u * 5 + 1));
}

TEST_CASE("try-debugf", "run")
{
    REQUIRE_NOTHROW(vle::debugf("%s%s%s%s%s",
                                str500.c_str(),
                                str500.c_str(),
                                str500.c_str(),
                                str500.c_str(),
                                str500.c_str()));
}
