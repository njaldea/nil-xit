// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#include <nil/xit/tagged/on_load.hpp>

#include "structs.hpp"

namespace nil::xit::tagged
{
    void on_load(Frame& frame, std::function<void(std::string_view)> callback)
    {
        frame.on_load = std::move(callback);
    }
}
