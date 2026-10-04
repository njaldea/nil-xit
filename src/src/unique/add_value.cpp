// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#include <nil/xit/unique/add_value.hpp>

#include "structs.hpp"

namespace nil::xit::unique::impl
{
    Value<std::vector<std::uint8_t>>& add_value(
        Frame& frame,
        std::string id,
        std::unique_ptr<IAccessor<std::vector<std::uint8_t>>> accessor
    )
    {
        return frame.values
            .emplace(id, Value<std::vector<std::uint8_t>>{&frame, id, std::move(accessor)})
            .first->second;
    }
}
