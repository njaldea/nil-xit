#pragma once

#include "../structs.hpp"

#include <filesystem>
#include <string>

namespace nil::xit
{
    Frame& add_frame(Core& core, std::string id, std::filesystem::path path);
    Frame& add_frame(
        const std::unique_ptr<Core, void (*)(Core*)>& core_ptr,
        std::string id,
        std::filesystem::path path
    );
}
