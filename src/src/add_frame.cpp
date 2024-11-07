#include <nil/xit/add_frame.hpp>

#include "structs.hpp"

namespace nil::xit
{
    unique::Frame& add_unique_frame(
        Core& core,
        std::string id,
        std::filesystem::path path,
        std::function<void()> on_load
    )
    {
        auto f = unique::Frame{&core, id, std::move(path), std::move(on_load), {}, {}, {}};
        return std::get<unique::Frame>(
            core.frames.emplace(std::move(id), std::move(f)).first->second
        );
    }

    tagged::Frame& add_tagged_frame(
        Core& core,
        std::string id,
        std::filesystem::path path,
        std::function<void(std::string_view)> on_load
    )
    {
        auto f = tagged::Frame{&core, id, std::move(path), std::move(on_load), {}, {}, {}};
        return std::get<tagged::Frame>(
            core.frames.emplace(std::move(id), std::move(f)).first->second
        );
    }
}
