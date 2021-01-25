//*  _                            _     _                      _             _      *
//* (_)_ __ ___  _ __   ___  _ __| |_  | |_ ___ _ __ _ __ ___ (_)_ __   __ _| |___  *
//* | | '_ ` _ \| '_ \ / _ \| '__| __| | __/ _ \ '__| '_ ` _ \| | '_ \ / _` | / __| *
//* | | | | | | | |_) | (_) | |  | |_  | ||  __/ |  | | | | | | | | | | (_| | \__ \ *
//* |_|_| |_| |_| .__/ \___/|_|   \__|  \__\___|_|  |_| |_| |_|_|_| |_|\__,_|_|___/ *
//*             |_|                                                                 *
//===- include/pstore/exchange/import_terminals.hpp -----------------------===//
//
// Part of the pstore project, under the Apache License v2.0 with LLVM Exceptions.
// See https://github.com/SNSystems/pstore/blob/master/LICENSE.txt for license
// information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef PSTORE_EXCHANGE_IMPORT_TERMINALS_HPP
#define PSTORE_EXCHANGE_IMPORT_TERMINALS_HPP

#include "pstore/exchange/import_rule.hpp"

namespace pstore {
    namespace exchange {
        namespace import {

            class uint64_rule final : public rule {
            public:
                uint64_rule (not_null<context *> const ctxt,
                             not_null<std::uint64_t *> const v) noexcept
                        : rule (ctxt)
                        , v_{v} {}
                std::error_code uint64_value (std::uint64_t v) override;
                gsl::czstring name () const noexcept override;

            private:
                not_null<std::uint64_t *> const v_;
            };

            class string_rule final : public rule {
            public:
                string_rule (not_null<context *> const ctxt,
                             not_null<std::string *> const v) noexcept
                        : rule (ctxt)
                        , v_{v} {}
                std::error_code string_value (std::string const & v) override;
                gsl::czstring name () const noexcept override;

            private:
                not_null<std::string *> const v_;
            };

        } // end namespace import
    }     // end namespace exchange
} // end namespace pstore

#endif // PSTORE_EXCHANGE_IMPORT_TERMINALS_HPP
