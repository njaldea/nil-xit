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
