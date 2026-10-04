// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#include <nil/xit/tagged/add_option.hpp>

#include "structs.hpp"

namespace nil::xit::tagged
{
    void add_option(Frame& frame, std::string key, std::string value)
    {
        frame.options.emplace(std::move(key), std::move(value));
    }
}
