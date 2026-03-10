#include <nil/xit/unique/post.hpp>

#include "../codec.hpp"
#include "../structs.hpp"
#include "utils.hpp"

namespace nil::xit::unique
{
    auto& get_fid(const auto& value)
    {
        return value.frame->subscribers;
    }

    void post(const Value<bool>& value, bool new_value)
    {
        value.frame->core->run_service->dispatch(
            [value = &value, new_value]()
            { post_impl(*value, new_value, value->frame->subscribers); }
        );
    }

    void post(const Value<double>& value, double new_value)
    {
        value.frame->core->run_service->dispatch(
            [value = &value, new_value]()
            { post_impl(*value, new_value, value->frame->subscribers); }
        );
    }

    void post(const Value<std::int64_t>& value, std::int64_t new_value)
    {
        value.frame->core->run_service->dispatch(
            [value = &value, new_value]()
            { post_impl(*value, new_value, value->frame->subscribers); }
        );
    }

    void post(const Value<std::string>& value, std::string new_value)
    {
        value.frame->core->run_service->dispatch(
            [value = &value, new_value = std::move(new_value)]() mutable
            { post_impl(*value, std::move(new_value), value->frame->subscribers); }
        );
    }

    void post(const Value<std::vector<std::uint8_t>>& value, std::vector<std::uint8_t> new_value)
    {
        value.frame->core->run_service->dispatch(
            [value = &value, new_value = std::move(new_value)]() mutable
            { post_impl(*value, std::move(new_value), value->frame->subscribers); }
        );
    }
}
