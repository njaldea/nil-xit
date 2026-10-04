// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#include <nil/xit/add_frame.hpp>

#include "structs.hpp"

namespace nil::xit
{
    unique::Frame& add_unique_frame(Core& core, std::string id)
    {
        auto f = unique::Frame{&core, id, std::nullopt, {}, {}, {}, {}, {}, {}};
        return core.unique_frames.emplace(std::move(id), std::move(f)).first->second;
    }

    unique::Frame& add_unique_frame(Core& core, std::string id, FileInfo file_info)
    {
        auto f = unique::Frame{&core, id, std::move(file_info), {}, {}, {}, {}, {}, {}};
        return core.unique_frames.emplace(std::move(id), std::move(f)).first->second;
    }

    tagged::Frame& add_tagged_frame(Core& core, std::string id)
    {
        auto f = tagged::Frame{&core, id, std::nullopt, {}, {}, {}, {}, {}, {}};
        return core.tagged_frames.emplace(std::move(id), std::move(f)).first->second;
    }

    tagged::Frame& add_tagged_frame(Core& core, std::string id, FileInfo file_info)
    {
        auto f = tagged::Frame{&core, id, std::move(file_info), {}, {}, {}, {}, {}, {}};
        return core.tagged_frames.emplace(std::move(id), std::move(f)).first->second;
    }
}
