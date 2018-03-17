//*      _        _                           _   _      *
//*  ___| |_ __ _| |_ _   _ ___   _ __   __ _| |_| |__   *
//* / __| __/ _` | __| | | / __| | '_ \ / _` | __| '_ \  *
//* \__ \ || (_| | |_| |_| \__ \ | |_) | (_| | |_| | | | *
//* |___/\__\__,_|\__|\__,_|___/ | .__/ \__,_|\__|_| |_| *
//*                              |_|                     *
//===- include/pstore/broker_intf/status_path.hpp -------------------------===//
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

#ifndef PSTORE_BROKER_INTF_STATUS_PATH_HPP
#define PSTORE_BROKER_INTF_STATUS_PATH_HPP (1)

#include <string>

#ifndef _WIN32
#include <cerrno>
#include <netinet/in.h>
#else
#include <Winsock2.h>
using in_port_t = unsigned short;
#endif

namespace pstore {
    namespace broker {

        std::string get_status_path ();

        inline int get_last_error () noexcept {
#ifndef _WIN32
            return errno;
#else
            return WSAGetLastError ();
#endif
        }

        void write_port_number_file (std::string const & path, in_port_t port);
        in_port_t read_port_number_file (std::string const & path);

    } // namespace broker
} // namespace pstore

#endif // PSTORE_BROKER_INTF_STATUS_PATH_HPP
// eof: include/pstore/broker_intf/status_path.hpp