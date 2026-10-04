// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#include <nil/xit/unique/on_sub.hpp>

#include "structs.hpp"

namespace nil::xit::unique
{
    void on_sub(Frame& frame, std::function<void(std::size_t)> callback)
    {
        frame.on_sub = std::move(callback);
    }
}
