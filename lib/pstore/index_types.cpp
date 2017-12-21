//*  _           _             _                          *
//* (_)_ __   __| | _____  __ | |_ _   _ _ __   ___  ___  *
//* | | '_ \ / _` |/ _ \ \/ / | __| | | | '_ \ / _ \/ __| *
//* | | | | | (_| |  __/>  <  | |_| |_| | |_) |  __/\__ \ *
//* |_|_| |_|\__,_|\___/_/\_\  \__|\__, | .__/ \___||___/ *
//*                                |___/|_|               *
//===- lib/pstore/index_types.cpp -----------------------------------------===//
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

#include "pstore/index_types.hpp"

#include "pstore/transaction.hpp"
#include "pstore/hamt_map.hpp"
#include "pstore/hamt_set.hpp"

namespace {

    // get_index
    // ~~~~~~~~~
    template <typename IndexType>
    IndexType * get_index (pstore::database & db, enum pstore::trailer::indices which,
                           bool create) {
        auto & dx = db.get_index (which);

        // Have we already loaded this index?
        if (dx.get () == nullptr) {
            std::shared_ptr<pstore::trailer const> footer = db.get_footer ();
            pstore::address const location = footer->a.index_records.at (
                static_cast<std::underlying_type<decltype (which)>::type> (which));
            if (location == pstore::address::null ()) {
                if (create) {
                    // Create a new (empty) index.
                    dx = std::make_unique<IndexType> (db);
                }
            } else {
                // Construct the index from the location.
                dx = std::make_unique<IndexType> (db, location);
            }
        }

#ifdef PSTORE_CPP_RTTI
        assert ((!create && dx.get () == nullptr) ||
                dynamic_cast<IndexType *> (dx.get ()) != nullptr);
#endif
        return static_cast<IndexType *> (dx.get ());
    }
}

namespace pstore {
    namespace index {

        // get_write_index
        // ~~~~~~~~~~~~~~~
        write_index * get_write_index (database & db, bool create) {
            return get_index<write_index> (db, trailer::indices::write, create);
        }

        // get_digest_index
        // ~~~~~~~~~~~~~~~~
        digest_index * get_digest_index (database & db, bool create) {
            return get_index<digest_index> (db, trailer::indices::digest, create);
        }

        // get_ticket_index
        // ~~~~~~~~~~~~~~~~
        pstore::index::ticket_index * get_ticket_index (database & db, bool create) {
            return get_index<ticket_index> (db, trailer::indices::ticket, create);
        }

        // get_name_index
        // ~~~~~~~~~~~~~~
        name_index * get_name_index (database & db, bool create) {
            return get_index<name_index> (db, trailer::indices::name, create);
        }

        // flush_indices
        // ~~~~~~~~~~~~~
        void flush_indices (::pstore::transaction_base & transaction,
                            trailer::index_records_array * const locations) {
            pstore::database & db = transaction.db ();
            if (index::write_index * const write = get_write_index (db, false /*create*/)) {
                (*locations)[trailer::indices::write] = write->flush (transaction);
            }

            if (index::digest_index * const digest = get_digest_index (db, false /*create*/)) {
                (*locations)[trailer::indices::digest] = digest->flush (transaction);
            }

            if (index::ticket_index * const ticket = get_ticket_index (db, false /*create*/)) {
                (*locations)[trailer::indices::ticket] = ticket->flush (transaction);
            }

            if (index::name_index * const name = get_name_index (db, false /*create*/)) {
                (*locations)[trailer::indices::name] = name->flush (transaction);
            }

            assert (locations->size () == trailer::indices::last);
        }
    } // namespace index
} // namespace pstore
// eof: lib/pstore/index_types.cpp