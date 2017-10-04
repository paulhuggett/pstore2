//*              *
//*   __ _  ___  *
//*  / _` |/ __| *
//* | (_| | (__  *
//*  \__, |\___| *
//*  |___/       *
//===- lib/broker/gc_posix.cpp --------------------------------------------===//
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
/// \file gc_posix.cpp
/// \brief Waits for garbage-collection processes to exit. On exit, a process is removed from the
/// gc_watch_thread::processes_ collection.

#include "broker/gc.hpp"

#ifndef _WIN32

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "pstore_support/logging.hpp"
#include "pstore_support/signal_cv.hpp"

#include "broker/globals.hpp"

namespace {

    char const * core_dump_string (int status) {
#ifdef WCOREDUMP
        if (WCOREDUMP (status)) {
            return " (core file generated)";
        }
#endif
        return "";
    }

    void pr_exit (pid_t pid, int status) {
        if (WIFEXITED (status)) {
            pstore::logging::log (pstore::logging::priority::info, "GC process ", pid,
                                  " exited. Normal termination, exit status = ",
                                  WEXITSTATUS (status));
        } else if (WIFSIGNALED (status)) {
            pstore::logging::log (pstore::logging::priority::error, "GC process ", pid,
                                  " exited. Abormal termination, signal number ", WTERMSIG (status),
                                  core_dump_string (status));
        } else if (WIFSTOPPED (status)) {
            pstore::logging::log (pstore::logging::priority::info, "GC process ", pid,
                                  " exited. child stopped, signal number = ", WSTOPSIG (status));
        }
    }

} // (anonymous namespace)

namespace broker {

    // watcher
    // ~~~~~~~
    void gc_watch_thread::watcher () {
        pstore::logging::log (pstore::logging::priority::info, "starting gc process watch thread");

        std::unique_lock<decltype (mut_)> lock (mut_);
        while (!done) {
            pstore::logging::log (pstore::logging::priority::info,
                                  "waiting for a GC process to complete");
            cv_.wait (lock);
            // We may have been woken up because the program is exiting.
            if (done) {
                continue;
            }

            // Loop to ensure that we query all processes that exited whilst we were waiting.
            auto status = 0;
            while (auto const pid = ::waitpid (-1, &status, WUNTRACED | WNOHANG)) {
                if (pid == -1) {
                    int const err = errno;
                    // If the error was "no child processes", we shouldn't report it.
                    if (err != ECHILD) {
                        char errbuf[256];
                        ::strerror_r (err, errbuf, sizeof (errbuf));
                        errbuf[255] = '\0';
                        pstore::logging::log (pstore::logging::priority::error, "waitpid error ",
                                              err, "(", errbuf, ")");
                    }
                    break;
                } else {
                    pr_exit (pid, status);
                    processes_.eraser (pid);
                }
            }
        }

        // FIXME: if an exception is thrown we should probably still clean up proceses.

        // Ask any child GC processes to quit.
        pstore::logging::log (pstore::logging::priority::info, "cleaning up");
        for (auto it = processes_.right_begin (), end = processes_.right_end (); it != end; ++it) {
            auto pid = *it;
            pstore::logging::log (pstore::logging::priority::info, "sending SIGINT to ", pid);
            ::kill (pid, SIGINT);
        }
    }

    // child_signal
    // ~~~~~~~~~~~~
    /// \note This function is called from a signal handler so muyst restrict itself to signal-safe
    /// functions.
    void gc_watch_thread::child_signal (int sig) noexcept {
        cv_.notify (sig);
    }

} // namespace broker
#endif // _!WIN32
// eof: lib/broker/gc_posix.cpp
