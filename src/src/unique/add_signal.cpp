// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#include <nil/xit/unique/add_signal.hpp>

#include "structs.hpp"

#include <functional>
#include <string>

namespace nil::xit::unique::impl
{
    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::span<const std::uint8_t>)> callback
    )
    {
        frame.signals.emplace(
            std::move(id),
            Signal<std::span<const std::uint8_t>>(std::move(callback))
        );
    }
}
