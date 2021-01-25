//*                       __      *
//*  _ __ ___  _ __ ___  / _|___  *
//* | '__/ _ \| '_ ` _ \| |_/ __| *
//* | | | (_) | | | | | |  _\__ \ *
//* |_|  \___/|_| |_| |_|_| |___/ *
//*                               *
//===- include/pstore/romfs/romfs.hpp -------------------------------------===//
//
// Part of the pstore project, under the Apache License v2.0 with LLVM Exceptions.
// See https://github.com/SNSystems/pstore/blob/master/LICENSE.txt for license
// information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef PSTORE_ROMFS_ROMFS_HPP
#define PSTORE_ROMFS_ROMFS_HPP

#include <cerrno>

#include "pstore/romfs/directory.hpp"
#include "pstore/romfs/dirent.hpp"

namespace pstore {
    namespace romfs {

        enum class error_code : int {
            einval = static_cast<int> (std::errc::invalid_argument),
            enoent = static_cast<int> (std::errc::no_such_file_or_directory),
            enotdir = static_cast<int> (std::errc::not_a_directory),
        };

        class error_category : public std::error_category {
        public:
            // The need for this constructor was removed by CWG defect 253 but Clang (prior
            // to 3.9.0) and GCC (before 4.6.4) require its presence.
            error_category () noexcept {} // NOLINT
            char const * PSTORE_NONNULL name () const noexcept override;
            std::string message (int error) const override;
        };

        error_category const & get_romfs_error_category () noexcept;
        std::error_code make_error_code (pstore::romfs::error_code e);

    } // end namespace romfs
} // end namespace pstore

namespace std {

    template <>
    struct is_error_code_enum<pstore::romfs::error_code> : std::true_type {};

} // namespace std

namespace pstore {
    namespace romfs {
        class open_file;
        class open_directory;

        //*     _                _      _            *
        //*  __| |___ ___ __ _ _(_)_ __| |_ ___ _ _  *
        //* / _` / -_|_-</ _| '_| | '_ \  _/ _ \ '_| *
        //* \__,_\___/__/\__|_| |_| .__/\__\___/_|   *
        //*                       |_|                *
        class descriptor {
            friend class romfs;

        public:
            descriptor (descriptor const & other) = default;
            descriptor (descriptor && other) = default;
            ~descriptor () noexcept = default;

            descriptor & operator= (descriptor const & other) = default;
            descriptor & operator= (descriptor && other) = default;

            std::size_t read (void * PSTORE_NONNULL buffer, std::size_t size, std::size_t count);
            error_or<std::size_t> seek (off_t offset, int whence);
            struct stat stat () const;

        private:
            explicit descriptor (std::shared_ptr<open_file> f)
                    : f_{std::move (f)} {}
            // Using a shared_ptr<> here so that descriptor instances can be passed around in
            // the same way as they would if 'descriptor' was the int type that's traditionally
            // used to represent file descriptors.
            std::shared_ptr<open_file> f_;
        };

        //*     _ _             _        _                _      _            *
        //*  __| (_)_ _ ___ _ _| |_   __| |___ ___ __ _ _(_)_ __| |_ ___ _ _  *
        //* / _` | | '_/ -_) ' \  _| / _` / -_|_-</ _| '_| | '_ \  _/ _ \ '_| *
        //* \__,_|_|_| \___|_||_\__| \__,_\___/__/\__|_| |_| .__/\__\___/_|   *
        //*                                                |_|                *
        class dirent_descriptor {
            friend class romfs;

        public:
            dirent const * PSTORE_NULLABLE read ();
            void rewind ();

        private:
            explicit dirent_descriptor (std::shared_ptr<open_directory> f)
                    : f_{std::move (f)} {}
            std::shared_ptr<open_directory> f_;
        };


        //*                 __     *
        //*  _ _ ___ _ __  / _|___ *
        //* | '_/ _ \ '  \|  _(_-< *
        //* |_| \___/_|_|_|_| /__/ *
        //*                        *
        class romfs {
        public:
            explicit romfs (directory const * PSTORE_NONNULL root);
            romfs (romfs const &) noexcept = default;
            romfs (romfs &&) noexcept = default;
            ~romfs () noexcept = default;

            romfs & operator= (romfs const &) = delete;
            romfs & operator= (romfs &&) = delete;

            error_or<descriptor> open (gsl::czstring PSTORE_NONNULL path) const;
            error_or<dirent_descriptor> opendir (gsl::czstring PSTORE_NONNULL path);
            error_or<struct stat> stat (gsl::czstring PSTORE_NONNULL path) const;

            error_or<std::string> getcwd () const;
            std::error_code chdir (gsl::czstring PSTORE_NONNULL path);

            /// \brief Check that the file system's structures are intact.
            ///
            /// Since the data is read-only there should be no need to call this function except as
            /// a belf-and-braces debug check.
            bool fsck () const;

        private:
            using dirent_ptr = dirent const * PSTORE_NONNULL;

            error_or<std::string> dir_to_string (directory const * const PSTORE_NONNULL dir) const;

            static dirent const * PSTORE_NONNULL
            directory_to_dirent (directory const * PSTORE_NONNULL d);

            static directory const * PSTORE_NONNULL
            dirent_to_directory (dirent const * PSTORE_NONNULL de);

            error_or<dirent_ptr> parse_path (gsl::czstring const PSTORE_NONNULL path) const {
                return parse_path (path, cwd_);
            }

            /// Parse a path string returning the directory-entry to which it refers or an error.
            /// Paths follow the POSIX convention of using a slash ('/') to separate components. A
            /// leading slash indicates that the search should start at the file system's root
            /// directory rather than the default directory given by the \p start_dir argument.
            ///
            /// \param path The path string to be parsed.
            /// \param start_dir  The directory to which the path is relative. Ignored if the
            /// initial character of the \p path argument is a slash.
            /// \returns  The directory entry described by the \p path argument or an error if the
            /// string was not valid.
            error_or<dirent_ptr> parse_path (gsl::czstring PSTORE_NONNULL path,
                                             directory const * PSTORE_NONNULL start_dir) const;


            directory const * const PSTORE_NONNULL root_;
            directory const * PSTORE_NONNULL cwd_;
        };

    } // end namespace romfs
} // end namespace pstore

#endif // PSTORE_ROMFS_ROMFS_HPP
