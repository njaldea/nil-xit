#include <nil/xit/methods/add_frame.hpp>

#include "../structs.hpp"

namespace nil::xit
{
    Frame& add_frame(Core& core, std::string id, std::filesystem::path path)
    {
        auto f = Frame{&core, id, std::move(path), {}};
        return core.frames.emplace(std::move(id), std::move(f)).first->second;
    }
}
