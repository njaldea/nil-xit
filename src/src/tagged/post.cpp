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

    void post(const Value<bool>& value, std::string tag, bool new_value)
    {
        value.frame->core->run_service->dispatch(
            [tag = std::move(tag), value = &value, new_value]()
            { post_impl(tag, *value, new_value, get_fid(value, tag)); }
        );
    }

    void post(const Value<double>& value, std::string tag, double new_value)
    {
        value.frame->core->run_service->dispatch(
            [tag = std::move(tag), value = &value, new_value]()
            { post_impl(tag, *value, new_value, get_fid(value, tag)); }
        );
    }

    void post(const Value<std::int64_t>& value, std::string tag, std::int64_t new_value)
    {
        value.frame->core->run_service->dispatch(
            [tag = std::move(tag), value = &value, new_value]()
            { post_impl(tag, *value, new_value, get_fid(value, tag)); }
        );
    }

    void post(const Value<std::string>& value, std::string tag, std::string new_value)
    {
        value.frame->core->run_service->dispatch(
            [tag = std::move(tag), value = &value, new_value = std::move(new_value)]() mutable
            { post_impl(tag, *value, std::move(new_value), get_fid(value, tag)); }
        );
    }

    void post(
        const Value<std::vector<std::uint8_t>>& value,
        std::string tag,
        std::vector<std::uint8_t> new_value
    )
    {
        value.frame->core->run_service->dispatch(
            [tag = std::move(tag), value = &value, new_value = std::move(new_value)]() mutable
            { post_impl(tag, *value, std::move(new_value), get_fid(value, tag)); }
        );
    }
}
