// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#pragma once

#include "structs.hpp"

#include <functional>

namespace nil::xit::unique
{
    void on_sub(Frame& frame, std::function<void(std::size_t)> callback);
}
