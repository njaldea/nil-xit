#include <nil/xit/tagged/post.hpp>

#include "structs.hpp"
#include "utils.hpp"

#include "../codec.hpp"
#include "../structs.hpp"

namespace nil::xit::tagged
{
    void post(std::string_view tag, const Value<bool>& value, bool new_value)
    {
        post_impl(
            tag,
            value,
            new_value,
            [&]()
            {
                auto _ = std::lock_guard(value.frame->core->mutex);
                return value.frame->subscribers[std::string(tag)];
            }()
        );
    }

    void post(std::string_view tag, const Value<double>& value, double new_value)
    {
        post_impl(
            tag,
            value,
            new_value,
            [&]()
            {
                auto _ = std::lock_guard(value.frame->core->mutex);
                return value.frame->subscribers[std::string(tag)];
            }()
        );
    }

    void post(std::string_view tag, const Value<std::int64_t>& value, std::int64_t new_value)
    {
        post_impl(
            tag,
            value,
            new_value,
            [&]()
            {
                auto _ = std::lock_guard(value.frame->core->mutex);
                return value.frame->subscribers[std::string(tag)];
            }()
        );
    }

    void post(std::string_view tag, const Value<std::string>& value, std::string new_value)
    {
        post_impl(
            tag,
            value,
            std::move(new_value),
            [&]()
            {
                auto _ = std::lock_guard(value.frame->core->mutex);
                return value.frame->subscribers[std::string(tag)];
            }()
        );
    }

    void post(
        std::string_view tag,
        const Value<std::vector<std::uint8_t>>& value,
        std::vector<std::uint8_t> new_value
    )
    {
        post_impl(
            tag,
            value,
            std::move(new_value),
            [&]()
            {
                auto _ = std::lock_guard(value.frame->core->mutex);
                return value.frame->subscribers[std::string(tag)];
            }()
        );
    }
}
