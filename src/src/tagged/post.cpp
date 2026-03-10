#include <nil/xit/tagged/post.hpp>

#include "../codec.hpp"
#include "../structs.hpp"
#include "utils.hpp"

namespace nil::xit::tagged
{
    auto get_fid(const auto* value, std::string_view tag) -> std::vector<nil::service::ID>
    {
        auto& subs = value->frame->subscribers;
        if (const auto it = subs.find(tag); it != subs.end())
        {
            return it->second;
        }
        return {};
    }

    void post(std::string tag, const Value<bool>& value, bool new_value)
    {
        value.frame->core->run_service->dispatch(
            [tag = std::move(tag), value = &value, new_value]()
            { post_impl(tag, *value, new_value, get_fid(value, tag)); }
        );
    }

    void post(std::string tag, const Value<double>& value, double new_value)
    {
        value.frame->core->run_service->dispatch(
            [tag = std::move(tag), value = &value, new_value]()
            { post_impl(tag, *value, new_value, get_fid(value, tag)); }
        );
    }

    void post(std::string tag, const Value<std::int64_t>& value, std::int64_t new_value)
    {
        value.frame->core->run_service->dispatch(
            [tag = std::move(tag), value = &value, new_value]()
            { post_impl(tag, *value, new_value, get_fid(value, tag)); }
        );
    }

    void post(std::string tag, const Value<std::string>& value, std::string new_value)
    {
        value.frame->core->run_service->dispatch(
            [tag = std::move(tag), value = &value, new_value = std::move(new_value)]() mutable
            { post_impl(tag, *value, std::move(new_value), get_fid(value, tag)); }
        );
    }

    void post(
        std::string tag,
        const Value<std::vector<std::uint8_t>>& value,
        std::vector<std::uint8_t> new_value
    )
    {
        value.frame->core->run_service->dispatch(
            [tag = std::move(tag), value = &value, new_value = std::move(new_value)]() mutable
            { post_impl(tag, *value, std::move(new_value), get_fid(value, tag)); }
        );
    }
}
