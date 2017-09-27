//*     _    _ _                      _  *
//*    / \  | (_) __ _ _ __   ___  __| | *
//*   / _ \ | | |/ _` | '_ \ / _ \/ _` | *
//*  / ___ \| | | (_| | | | |  __/ (_| | *
//* /_/   \_\_|_|\__, |_| |_|\___|\__,_| *
//*              |___/                   *
//===- include/pstore_mcrepo/Aligned.h ------------------------------------===//
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
#ifndef LLVM_MCREPOFRAGMENT_ALIGNED_H
#define LLVM_MCREPOFRAGMENT_ALIGNED_H

#include <cassert>
#include <cstdint>
#include <type_traits>

namespace pstore {
    namespace repo {

        /// \returns True if the input value is a power of 2.
        template <typename Ty, typename = typename std::enable_if<std::is_unsigned<Ty>::value>>
        inline bool isPowerOfTwo (Ty n) {
            //  if a number n is a power of 2 then bitwise & of n and n-1 will be zero.
            return n && !(n & (n - 1U));
        }

        /// \param V  The value to be aligned.
        /// \param Align  The alignment required for 'v'.
        /// \returns  The value closest to but greater than or equal to 'V' for which
        /// V%Align is zero.
        template <typename IntType>
        inline IntType aligned (IntType V, std::size_t Align) {
            assert (isPowerOfTwo (Align));
            return (V + Align - 1U) & ~(Align - 1U);
        }

        /// \param V  The value to be aligned.
        /// \returns  The value closest to but greater than or equal to 'V' for which
        /// V%alignof(decltype(V)) is zero.
        template <typename AlignType, typename IntType,
                  typename = std::enable_if<std::is_integral<IntType>::value>>
        inline IntType aligned (IntType V) {
            return aligned (V, alignof (AlignType));
        }

        template <typename PointeeType>
        inline PointeeType * aligned (void * v) {
            auto p = reinterpret_cast<std::uintptr_t> (v);
            p = aligned (p, alignof (PointeeType));
            return reinterpret_cast<PointeeType *> (p);
        }
        template <typename PointeeType>
        inline PointeeType const * aligned (void const * v) {
            return aligned<PointeeType> (const_cast<void *> (v));
        }

        template <typename DestPointeeType, typename SrcPointeeType = DestPointeeType>
        inline DestPointeeType * alignedPtr (SrcPointeeType * v) {
            return aligned<DestPointeeType> (reinterpret_cast<void *> (v));
        }
        template <typename DestPointeeType, typename SrcPointeeType = DestPointeeType>
        inline DestPointeeType const * alignedPtr (SrcPointeeType const * p) {
            auto v = reinterpret_cast<void const *> (p);
            return aligned<DestPointeeType> (const_cast<void *> (v));
        }

    } // end namespace repo
} // end namespace pstore

#endif // LLVM_MCREPOFRAGMENT_ALIGNED_H
// eof: include/pstore_mcrepo/Aligned.h
