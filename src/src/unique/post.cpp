#include <nil/xit/unique/post.hpp>

#include "structs.hpp"
#include "utils.hpp"

#include "../codec.hpp"
#include "../structs.hpp"

#include <nil/service.hpp>

namespace nil::xit::unique
{
    void post(const Value<bool>& value, bool new_value)
    {
        post_impl(
            value,
            new_value,
            [&]()
            {
                auto _ = std::lock_guard(value.frame->core->mutex);
                return value.frame->subscribers;
            }()
        );
    }

    void post(const Value<double>& value, double new_value)
    {
        post_impl(
            value,
            new_value,
            [&]()
            {
                auto _ = std::lock_guard(value.frame->core->mutex);
                return value.frame->subscribers;
            }()
        );
    }

    void post(const Value<std::int64_t>& value, std::int64_t new_value)
    {
        post_impl(
            value,
            new_value,
            [&]()
            {
                auto _ = std::lock_guard(value.frame->core->mutex);
                return value.frame->subscribers;
            }()
        );
    }

    void post(const Value<std::string>& value, std::string new_value)
    {
        post_impl(
            value,
            std::move(new_value),
            [&]()
            {
                auto _ = std::lock_guard(value.frame->core->mutex);
                return value.frame->subscribers;
            }()
        );
    }

    void post(const Value<std::vector<std::uint8_t>>& value, std::vector<std::uint8_t> new_value)
    {
        post_impl(
            value,
            std::move(new_value),
            [&]()
            {
                auto _ = std::lock_guard(value.frame->core->mutex);
                return value.frame->subscribers;
            }()
        );
    }
}
