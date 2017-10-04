//*        _     _           _    *
//*   ___ | |__ (_) ___  ___| |_  *
//*  / _ \| '_ \| |/ _ \/ __| __| *
//* | (_) | |_) | |  __/ (__| |_  *
//*  \___/|_.__// |\___|\___|\__| *
//*           |__/                *
//===- unittests/dump/test_object.cpp -------------------------------------===//
// Copyright (c) 2017 by Sony Interactive Entertainment, Inc.
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
#include "dump/value.hpp"
// System includes
#include <sstream>
// 3rd partly includes
#include "gtest/gtest.h"
#include "gmock/gmock.h"
// Local includes
#include "split.hpp"
#include "convert.hpp"

namespace {
    template <typename CharType>
    class Object : public ::testing::Test {
    public:
        void SetUp () final {
        }
        void TearDown () final {
        }
    protected:
        std::basic_ostringstream <CharType> out;
    };
    
}

typedef ::testing::Types <char, wchar_t>  CharacterTypes;
TYPED_TEST_CASE (Object, CharacterTypes);

TYPED_TEST (Object, Empty) {
    value::object v;
    v.write (this->out);
    auto const & actual = this->out.str ();
    auto const & expected = convert <TypeParam> ("{ }");
    EXPECT_EQ (expected, actual);
}

TYPED_TEST (Object, SingleNumber) {
    value::object v{value::object::container{{"key", value::make_number (42)}}};
    v.write (this->out);
    auto const & actual = this->out.str ();
    EXPECT_THAT (split_tokens (actual),
                 ::testing::ElementsAre (convert <TypeParam> ("key"),
                                         convert <TypeParam> (":"),
                                         convert <TypeParam> ("0x2a")));
}

TYPED_TEST (Object, TwoNumbers) {
    value::object v{value::object::container{
        {"k1", value::make_number (42)}, {"k2", value::make_number (43)},
    }};
    v.write (this->out);

    auto const lines = split_lines (this->out.str ());
    ASSERT_EQ (2U, lines.size ());
    EXPECT_THAT (split_tokens (lines.at (0)),
                 ::testing::ElementsAre (convert <TypeParam> ("k1"),
                                         convert <TypeParam> (":"),
                                         convert <TypeParam> ("0x2a")));
    EXPECT_THAT (split_tokens (lines.at (1)),
                 ::testing::ElementsAre (convert <TypeParam> ("k2"),
                                         convert <TypeParam> (":"),
                                         convert <TypeParam> ("0x2b")));
}

TYPED_TEST (Object, KeyWithColon) {
    value::object v{value::object::container{
        {"k1:k2", value::make_number (42)},
    }};
    v.write (this->out);

    auto const actual = this->out.str ();
    EXPECT_THAT (split_tokens (actual),
                 ::testing::ElementsAre (convert <TypeParam> ("k1:k2"),
                                         convert <TypeParam> (":"),
                                         convert <TypeParam> ("0x2a")));
}

TYPED_TEST (Object, KeyWithColonSpace) {
    value::object v{{
        {"k1: k2", value::make_number (42)},
    }};
    v.write (this->out);
    auto const actual = this->out.str ();
    auto const expected = convert <TypeParam> ("\"k1: k2\" : 0x2a");
    EXPECT_EQ (expected, actual);
}

TYPED_TEST (Object, KeyNeedingQuoting) {
    value::object v{{
        {"  k1", value::make_number (42)},
    }};
    v.write (this->out);
    auto const actual   = this->out.str ();
    auto const expected = convert <TypeParam> ("\"  k1\" : 0x2a");
    EXPECT_EQ (expected, actual);
}

TYPED_TEST (Object, ValueAlignment) {
    value::object v{{
        {"short", value::make_number (42)}, {"much_longer", value::make_number (43)},
    }};
    v.write (this->out);
    auto const actual   = this->out.str ();
    auto const expected = convert <TypeParam> ("short       : 0x2a\n"
                                               "much_longer : 0x2b");
    EXPECT_EQ (expected, actual);
}

TYPED_TEST (Object, Nested) {
    value::object v{{
        {"k1", value::make_value (std::string ("value1"))},
        {"k2", value::make_value (value::object::container{
                   {"ik1", value::make_value ("iv1")}, {"ik2", value::make_value ("iv2")},
               })},
    }};
    v.write (this->out);
    auto const actual = this->out.str ();
    auto const expected = convert <TypeParam> (
        "k1 : value1\n"
        "k2 : \n"
        "    ik1 : iv1\n"
        "    ik2 : iv2");
    EXPECT_EQ (expected, actual);
}

TEST (Object, GetFound) {
    auto v = value::make_value ("Hello World");
    value::object object{{{"key", v}}};
    EXPECT_EQ (v, object.get ("key"));
}

TEST (Object, GetNotFound) {
    auto v = value::make_value ("Hello World");
    value::object object{{{"key", v}}};
    EXPECT_EQ (std::shared_ptr<value::value> (nullptr), object.get ("missing"));
}

TEST (Object, BackInserter) {
    value::object object{{{"k1", value::make_value ("v1")}}};
}

// eof: unittests/dump/test_object.cpp
