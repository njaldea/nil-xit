#include <nil/xit/methods/add_frame.hpp>

#include "../structs.hpp"

namespace nil::xit
{
    Frame& add_frame(Core& core, std::string id, std::filesystem::path path)
    {
        auto f = Frame{&core, id, std::move(path), {}, {}};
        return std::get<Frame>( //
            core.frames.emplace(std::move(id), std::move(f)).first->second
        );
    }

    TaggedFrame& add_tagged_frame(Core& core, std::string id, std::filesystem::path path)
    {
        auto f = TaggedFrame{&core, id, std::move(path), {}, {}};
        return std::get<TaggedFrame>( //
            core.frames.emplace(std::move(id), std::move(f)).first->second
        );
    }
}
