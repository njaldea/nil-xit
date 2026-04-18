#include <nil/xit/tagged/post.hpp>

#include "../structs.hpp"
#include "utils.hpp"

namespace nil::xit::tagged::impl
{
    void post(
        const Value<std::vector<std::uint8_t>>& value,
        std::string tag,
        std::vector<std::uint8_t> new_value
    )
    {
        value.frame->core->run_service->dispatch(
            [tag = std::move(tag), value = &value, new_value = std::move(new_value)]() mutable
            {
                auto& subs = value->frame->subscribers;
                if (const auto it = subs.find(tag); it != subs.end())
                {
                    post_impl(tag, *value, std::move(new_value), it->second);
                    return;
                }
                post_impl(tag, *value, std::move(new_value), {});
            }
        );
    }
}
