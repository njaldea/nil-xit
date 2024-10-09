#pragma once

#include "../structs.hpp"

#include <filesystem>
#include <string>

namespace nil::xit
{
    Frame& add_frame(Core& core, std::string id, std::filesystem::path path);
    TaggedFrame& add_tagged_frame(Core& core, std::string id, std::filesystem::path path);
}
