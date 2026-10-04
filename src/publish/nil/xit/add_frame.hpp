// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#pragma once

#include "structs.hpp"

#include "tagged/structs.hpp"
#include "unique/structs.hpp"

#include <string>

namespace nil::xit
{
    unique::Frame& add_unique_frame(Core& core, std::string id);
    tagged::Frame& add_tagged_frame(Core& core, std::string id);

    unique::Frame& add_unique_frame(Core& core, std::string id, FileInfo file_info);
    tagged::Frame& add_tagged_frame(Core& core, std::string id, FileInfo file_info);
}
