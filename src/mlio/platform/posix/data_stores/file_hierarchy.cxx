/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not use this file except in compliance with the License. A copy of
 * the License is located at
 *
 *      http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#include "mlio/data_stores/file_hierarchy.h"  // IWYU pragma: associated

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <system_error>

#include <fnmatch.h>
#include <fts.h>
#include <sys/stat.h>

#include <fmt/format.h>

#include "mlio/config.h"
#include "mlio/data_stores/data_store.h"
#include "mlio/data_stores/file.h"
#include "mlio/detail/error.h"
#include "mlio/intrusive_ptr.h"

using mlio::detail::current_error_code;

namespace mlio {
inline namespace v1 {
namespace detail {
namespace {

struct FTS_deleter {
    void operator()(::FTS *fts)
    {
        if (fts != nullptr) {
            ::fts_close(fts);
        }
    }
};

std::vector<char const *>
get_pathname_c_strs(stdx::span<std::string const> pathnames)
{
    std::vector<char const *> c_strs;

    std::transform(pathnames.begin(), pathnames.end(),
                   std::back_inserter(c_strs), [](std::string const &pth)
    {
        return pth.c_str();
    });

    c_strs.push_back(nullptr);

    return c_strs;
}

#ifdef MLIO_PLATFORM_LINUX
#   define mlio_pathname_comparer ::strverscmp
#else
#   define mlio_pathname_comparer ::strcmp
#endif

inline int
ver_sort(::FTSENT const **a, ::FTSENT const **b)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
    return mlio_pathname_comparer((*a)->fts_name, (*b)->fts_name);
}

auto
make_fts(stdx::span<std::string const> pathnames)
{
    std::vector<char const *> c_strs = get_pathname_c_strs(pathnames);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    auto path_argv = const_cast<char * const *>(c_strs.data());

    ::FTS *fts = ::fts_open(path_argv, FTS_LOGICAL | FTS_NOCHDIR, ver_sort);
    if (fts == nullptr) {
        throw std::system_error{current_error_code(),
            "The specified pathnames cannot be traversed."};
    }

    return std::unique_ptr<::FTS, detail::FTS_deleter>(fts);
}

}  // namespace
}  // namespace detail

std::vector<intrusive_ptr<data_store>>
list_files(list_files_params const &prm)
{
    auto fts = detail::make_fts(prm.pathnames);

    std::vector<intrusive_ptr<data_store>> lst;

    ::FTSENT *e;
    while ((e = ::fts_read(fts.get())) != nullptr) {
        if (e->fts_info == FTS_ERR ||
            e->fts_info == FTS_DNR ||
            e->fts_info == FTS_NS) {

            std::error_code err = current_error_code();
            throw std::system_error{err, fmt::format(
                "The file or directory '{0}' cannot be opened.",
                e->fts_accpath)};
        }

        if (e->fts_info != FTS_F) {
            continue;
        }

        // We ignore anything but regular and block files.
        if (!S_ISREG(e->fts_statp->st_mode) &&
            !S_ISBLK(e->fts_statp->st_mode)) {

            continue;
        }

        std::string const *pattern = prm.pattern;
        if (pattern != nullptr && !pattern->empty()) {
            int r = ::fnmatch(pattern->c_str(), e->fts_accpath, 0);

            if (r == FNM_NOMATCH) {
                continue;
            }
            if (r != 0) {
                throw std::invalid_argument{
                    "The pattern cannot be used for comparison."};
            }
        }

        auto const *predicate = prm.predicate;
        if (predicate != nullptr && *predicate != nullptr) {
            if (!(*predicate)(e->fts_accpath)) {
                continue;
            }
        }

        lst.emplace_back(make_intrusive<file>(e->fts_accpath, prm.mmap, prm.cmp));
    }

    if (errno != 0) {
        throw std::system_error{current_error_code(),
            "The specified pathnames cannot be traversed."};
    }

    return lst;
}

}  // namespace v1
}  // namespace mlio
