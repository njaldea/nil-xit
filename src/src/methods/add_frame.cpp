#include <nil/xit/methods/add_frame.hpp>

#include "../structs.hpp"

namespace nil::xit
{
    Frame& add_frame(Core& core, std::string id, std::filesystem::path path)
    {
        auto f = Frame{&core, id, std::move(path), {}};
        return core.frames.emplace(std::move(id), std::move(f)).first->second;
    }

    Frame& add_frame(
        const std::unique_ptr<Core, void (*)(Core*)>& core_ptr,
        std::string id,
        std::filesystem::path path
    )
    {
        return add_frame(*core_ptr, std::move(id), std::move(path));
    }
}
