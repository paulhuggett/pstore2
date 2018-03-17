//*      _                              *
//*  ___| |_ ___  _ __ __ _  __ _  ___  *
//* / __| __/ _ \| '__/ _` |/ _` |/ _ \ *
//* \__ \ || (_) | | | (_| | (_| |  __/ *
//* |___/\__\___/|_|  \__,_|\__, |\___| *
//*                         |___/       *
//===- include/pstore/core/storage.hpp ------------------------------------===//
// Copyright (c) 2017-2018 by Sony Interactive Entertainment, Inc.
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
/// \file storage.cpp

#include <cassert>
#include <memory>

#include "pstore/core/address.hpp"
#include "pstore/core/make_unique.hpp"
#include "pstore/core/region.hpp"
#include "pstore/support/portab.hpp"

namespace pstore {
    template <typename Ty, typename = typename std::enable_if<std::is_unsigned<Ty>::value>>
    inline bool is_power_of_two (Ty n) {
        //  if a number n is a power of 2 then bitwise & of n and n-1 will be zero.
        return n && !(n & (n - 1U));
    }


    /// An entry in the segment address table.
    struct sat_entry {
        /// A pointer to the data belonging to the segment represented by this
        /// entry in the segment address table. The pointer will always lie
        /// within the memory-mapped region given by 'region'.
        std::shared_ptr<void> value;

        /// The memory-mapped region to which the 'value' pointer belongs.
        region::memory_mapper_ptr region;

#ifndef NDEBUG
        bool is_valid () const noexcept {
            if (value == nullptr && region == nullptr) {
                return true;
            }

            auto ptr = static_cast<std::uint8_t const *> (value.get ());
            auto region_base = static_cast<std::uint8_t const *> (region->data ().get ());
            auto region_end = region_base + region->size ();
            return ptr >= region_base && ptr + address::segment_size <= region_end;
        }
#endif
    };

    /// \brief The number of entries in the \ref segment_address_table.
    static auto constexpr sat_elements = 1U << address::segment_number_bits;
    using segment_address_table = std::array<sat_entry, sat_elements>;
    using sat_iterator = segment_address_table::iterator;
    using file_ptr = std::shared_ptr<file::file_base>;

    class storage {
    public:
        static auto constexpr full_region_size = UINT64_C (1) << 32; // 4 Gigabytes
        static auto constexpr min_region_size = UINT64_C (1) << 22;  // 4 Megabytes
        // Check that full_region_size is a multiple of min_region_size
        PSTORE_STATIC_ASSERT (full_region_size % min_region_size == 0);

        using region_container = std::vector<region::memory_mapper_ptr>;

        template <typename File>
        storage (std::shared_ptr<File> const & file,
                 std::unique_ptr<system_page_size_interface> && page_size,
                 std::unique_ptr<region::factory> && region_factory)
                : sat_{new segment_address_table}
                , file_{std::static_pointer_cast<file::file_base> (file)}
                , page_size_{std::move (page_size)}
                , region_factory_{std::move (region_factory)}
                , regions_{region_factory_->init ()} {}

        template <typename File>
        storage (std::shared_ptr<File> const & file)
                : sat_{new segment_address_table}
                , file_{std::static_pointer_cast<file::file_base> (file)}
                , page_size_{std::make_unique<system_page_size> ()}
                , region_factory_{region::get_factory (
                      std::static_pointer_cast<file::file_handle> (file), full_region_size,
                      min_region_size)}
                , regions_{region_factory_->init ()} {}

        file::file_base * file () { return file_.get (); }
        file::file_base const * file () const { return file_.get (); }

        void map_bytes (std::uint64_t new_size);

        /// Called to add newly created memory-mapped regions to the segment address table. This
        /// happens when the file is initially opened, and when it is grown by calling allocate().
        void update_master_pointers (std::size_t old_length);

        /// This function performs the bulk of the work of creating a "shadow" block when a request
        /// spans more than one memory-mapped region. It breaks the data into a series of copies
        /// (each read or writing as much data as possible) and calls the provided "copier" function
        /// to perform the actual copy. This same function is used to copy data from the store into
        /// a newly allocated block, and to copy from a contiguous block back to the store.
        ///
        /// The template traits argument controls the direction of copy: either
        /// copy_from_store_traits or copy_to_store_traits may be used.
        ///
        /// \param addr    The store address of the data to be copied.
        /// \param size    The number of bytes to be copied.
        /// \param p       A pointer to a block of memory of at least 'size' bytes.
        /// \param copier  A function that will perform the actual copy. The expected type of the
        ///                function's arguments are controlled by the 'traits' template argument.
        ///                The function will be passed 3 arguments:
        ///                - Traits::in_store_pointer: An address lying within the data store.
        ///                - Traits::temp_pointer: A pointer within the memory block given by
        ///                [p,p+size).
        ///                - std::uint64_t: The number of bytes to be copied.
        template <typename Traits, typename Function>
        void copy (address addr, std::size_t size, typename Traits::temp_pointer p,
                   Function copier) const;

        /// \brief Returns true if the given address range "spans" more than one region.
        ///
        /// \note The PSTORE_ALWAYS_SPANNING configure-time setting can cause this function to
        /// always return true.
        ///
        /// \param addr The start of the address range to be considered.
        /// \param size The size of the address range to be considered.
        /// \returns true if the given address range "spans" more than one region.
        bool request_spans_regions (address const & addr, std::size_t size) const;

        /// Marks the address range [first, last) as read-only.
        void protect (address first, address last);

        ///@{
        /// Returns the base address of a segment given its index.
        /// \param segment The segment number whose base address it to be returned. The segment
        ///                number must lie within the memory mapped regions.
        std::shared_ptr<void const> segment_base (address::segment_type segment) const;
        std::shared_ptr<void> segment_base (address::segment_type segment);
        ///@}
        std::shared_ptr<void> address_to_pointer (address const & sop);
        std::shared_ptr<void const> address_to_pointer (address const & sop) const;

        // For unit testing only.
        region_container const & regions () const { return regions_; }

    private:
        static sat_iterator slice_region_into_segments (std::shared_ptr<memory_mapper_base> region,
                                                        sat_iterator segment_it,
                                                        sat_iterator segment_end);

        /// The Segment Address Table: an array of pointers to the base-address of each segment's
        /// memory-mapped storage and their corresponding region object.
        std::unique_ptr<segment_address_table> sat_;

        /// The file used to hold the data.
        file_ptr file_;
        std::unique_ptr<system_page_size_interface> page_size_;
        std::unique_ptr<region::factory> region_factory_;
        region_container regions_;
    };

    // segment_base
    // ~~~~~~~~~~~~
    inline auto storage::segment_base (address::segment_type segment) const
        -> std::shared_ptr<void const> {
        assert (segment < sat_->size ());
        sat_entry const & e = (*sat_)[segment];
        assert (e.is_valid ());
        return e.value;
    }
    inline auto storage::segment_base (address::segment_type segment) -> std::shared_ptr<void> {
        auto const * cthis = this;
        return std::const_pointer_cast<void> (cthis->segment_base (segment));
    }

    // address
    // ~~~~~~~
    inline auto storage::address_to_pointer (address const & sop) const
        -> std::shared_ptr<void const> {
        std::shared_ptr<void const> segment_base = this->segment_base (sop.segment ());
        auto ptr =
            std::static_pointer_cast<std::uint8_t const> (segment_base).get () + sop.offset ();
        return std::shared_ptr<void const> (segment_base, ptr);
    }

    inline auto storage::address_to_pointer (address const & sop) -> std::shared_ptr<void> {
        auto const * cthis = this;
        return std::const_pointer_cast<void> (cthis->address_to_pointer (sop));
    }

    // copy
    // ~~~~
    template <typename Traits, typename Function>
    void storage::copy (address addr, std::size_t size, typename Traits::temp_pointer p,
                        Function copier) const {

        PSTORE_STATIC_ASSERT (std::numeric_limits<std::size_t>::max () <=
                              std::numeric_limits<std::uint64_t>::max ());
        address::segment_type segment = addr.segment ();
        assert (segment < sat_elements);
        sat_entry const & segment_pointer = (*sat_)[segment];

        auto in_store_ptr =
            static_cast<typename Traits::in_store_pointer> (segment_pointer.value.get ()) +
            addr.offset ();
        auto region_base =
            static_cast<typename Traits::in_store_pointer> (segment_pointer.region->data ().get ());
        assert (in_store_ptr >= region_base &&
                in_store_ptr <= region_base + segment_pointer.region->size ());

        std::uint64_t copy_size = segment_pointer.region->size () -
                                  static_cast<std::uintptr_t> (in_store_ptr - region_base);
        static_assert (std::numeric_limits<std::size_t>::max () <=
                           std::numeric_limits<std::uint64_t>::max (),
                       "size_t must not be larger than uint64!");
        copy_size = std::min (copy_size, std::uint64_t{size});

        // An initial copy for the tail of the first of the regions covered by the addr..addr+size
        // range.
        copier (in_store_ptr, p, copy_size);
        p += copy_size;
        size -= copy_size;

        // Now copy the subsequent region(s).
        while (size > 0) {
            // We copied all of the necessary data to the previous region. Now move to the next
            // region and do the same.
            std::uint64_t const inc =
                (copy_size + address::segment_size - 1) / address::segment_size;
            assert (inc < std::numeric_limits<address::segment_type>::max ());
            assert (segment + inc < sat_elements);
            segment += static_cast<address::segment_type> (inc);
            region::memory_mapper_ptr region = (*sat_)[segment].region;
            assert (region != nullptr);

            copy_size = std::min (static_cast<std::uint64_t> (size), region->size ());
            in_store_ptr = static_cast<typename Traits::in_store_pointer> (region->data ().get ());
            copier (in_store_ptr, p, copy_size);
            p += copy_size;
            size -= copy_size;
        }
    }

} // namespace pstore
// eof: include/pstore/core/storage.hpp