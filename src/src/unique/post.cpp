#include <nil/xit/unique/post.hpp>

#include "../structs.hpp"
#include "utils.hpp"

namespace nil::xit::unique::impl
{
    void post(const Value<std::vector<std::uint8_t>>& value, std::vector<std::uint8_t> new_value)
    {
        value.frame->core->run_service->dispatch(
            [value = &value, new_value = std::move(new_value)]() mutable
            { post_impl(*value, std::move(new_value), value->frame->subscribers); }
        );
    }
}
