//*                  _                      _                   *
//*  _ __ ___   __ _| | _____   _   _ _ __ (_) __ _ _   _  ___  *
//* | '_ ` _ \ / _` | |/ / _ \ | | | | '_ \| |/ _` | | | |/ _ \ *
//* | | | | | | (_| |   <  __/ | |_| | | | | | (_| | |_| |  __/ *
//* |_| |_| |_|\__,_|_|\_\___|  \__,_|_| |_|_|\__, |\__,_|\___| *
//*                                              |_|            *
//===- unittests/support/test_make_unique.cpp -----------------------------===//
// Copyright (c) 2017-2019 by Sony Interactive Entertainment, Inc.
// All rights reserved.
//
// Developed by:
//   Toolchain Team
//   SN Systems, Ltd.
//   www.snsystems.com
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal with the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// - Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimers.
//
// - Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimers in the
//   documentation and/or other materials provided with the distribution.
//
// - Neither the names of SN Systems Ltd., Sony Interactive Entertainment,
//   Inc. nor the names of its contributors may be used to endorse or
//   promote products derived from this Software without specific prior
//   written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
//===----------------------------------------------------------------------===//

#include "pstore/support/make_unique.hpp"
#include <gtest/gtest.h>

namespace {
    struct xtor_counter {
        xtor_counter () { ++ctor_calls; }
        ~xtor_counter () { ++dtor_calls; }

        static int ctor_calls;
        static int dtor_calls;
    };

    int xtor_counter::ctor_calls = 0;
    int xtor_counter::dtor_calls = 0;

    class MakeUnique : public ::testing::Test {
    public:
        MakeUnique () {
            xtor_counter::ctor_calls = 0;
            xtor_counter::dtor_calls = 0;
        }
    };
} // anonymous namespace

TEST_F (MakeUnique, Simple) {
    {
        auto ptr = pstore::make_unique<xtor_counter> ();
        EXPECT_NE (ptr.get (), nullptr);
    }
    EXPECT_EQ (xtor_counter::ctor_calls, 1);
    EXPECT_EQ (xtor_counter::dtor_calls, 1);
}

TEST_F (MakeUnique, Array) {
    {
        auto ptr = pstore::make_unique<xtor_counter[]> (5U);
        EXPECT_NE (ptr.get (), nullptr);
    }
    EXPECT_EQ (xtor_counter::ctor_calls, 5);
    EXPECT_EQ (xtor_counter::dtor_calls, 5);
}
