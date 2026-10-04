// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#pragma once

#include <nil/xit/structs.hpp>

#include "tagged/structs.hpp"
#include "unique/structs.hpp"

#include <nil/service/structs.hpp>
#include <nil/xalt/transparent_stl.hpp>

#include <filesystem>

namespace nil::xit
{
    struct Core
    {
        nil::service::IRunnableService* run_service;
        nil::service::IMessageService* msg_service;
        std::optional<std::filesystem::path> cache_location;
        nil::xalt::transparent_umap<std::filesystem::path> groups;
        nil::xalt::transparent_umap<unique::Frame> unique_frames;
        nil::xalt::transparent_umap<tagged::Frame> tagged_frames;
    };
}
