#pragma once

#include "structs.hpp"

#include "tagged/structs.hpp"
#include "unique/structs.hpp"

#include <filesystem>
#include <functional>
#include <string>

namespace nil::xit
{
    unique::Frame& add_unique_frame(
        Core& core,
        std::string id,
        std::function<void()> on_load = {} //
    );
    unique::Frame& add_unique_frame(
        Core& core,
        std::string id,
        std::filesystem::path path,
        std::function<void()> on_load = {}
    );
    tagged::Frame& add_tagged_frame(
        Core& core,
        std::string id,
        std::function<void(std::string_view)> on_load = {}
    );
    tagged::Frame& add_tagged_frame(
        Core& core,
        std::string id,
        std::filesystem::path path,
        std::function<void(std::string_view)> on_load = {}
    );
}
