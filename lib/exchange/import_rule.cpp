//*  _                            _                _       *
//* (_)_ __ ___  _ __   ___  _ __| |_   _ __ _   _| | ___  *
//* | | '_ ` _ \| '_ \ / _ \| '__| __| | '__| | | | |/ _ \ *
//* | | | | | | | |_) | (_) | |  | |_  | |  | |_| | |  __/ *
//* |_|_| |_| |_| .__/ \___/|_|   \__| |_|   \__,_|_|\___| *
//*             |_|                                        *
//===- lib/exchange/import_rule.cpp ---------------------------------------===//
// Copyright (c) 2017-2020 by Sony Interactive Entertainment, Inc.
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
#include "pstore/exchange/import_rule.hpp"
#include "pstore/exchange/import_error.hpp"

namespace pstore {
    namespace exchange {

        import_rule::~import_rule () = default;

        std::error_code import_rule::int64_value (std::int64_t const) {
            return import_error::unexpected_number;
        }
        std::error_code import_rule::uint64_value (std::uint64_t const) {
            return import_error::unexpected_number;
        }
        std::error_code import_rule::double_value (double const) {
            return import_error::unexpected_number;
        }
        std::error_code import_rule::boolean_value (bool const) {
            return import_error::unexpected_boolean;
        }
        std::error_code import_rule::null_value () { return import_error::unexpected_null; }
        std::error_code import_rule::begin_array () { return import_error::unexpected_array; }
        std::error_code import_rule::string_value (std::string const &) {
            return import_error::unexpected_string;
        }
        std::error_code import_rule::end_array () { return import_error::unexpected_end_array; }
        std::error_code import_rule::begin_object () { return import_error::unexpected_object; }
        std::error_code import_rule::key (std::string const &) {
            return import_error::unexpected_object_key;
        }
        std::error_code import_rule::end_object () { return import_error::unexpected_end_object; }

    } // end namespace exchange
} // end namespace pstore