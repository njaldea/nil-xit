#include <nil/xit/unique/post.hpp>

#include "structs.hpp"
#include "utils.hpp"

#include "../codec.hpp"
#include "../structs.hpp"

namespace nil::xit::unique
{
    auto& get_fid(const auto& value)
    {
        auto _ = std::lock_guard(value.frame->core->mutex);
        return value.frame->subscribers;
    }

    void post(const Value<bool>& value, bool new_value)
    {
        post_impl(value, new_value, get_fid(value));
    }

    void post(const Value<double>& value, double new_value)
    {
        post_impl(value, new_value, get_fid(value));
    }

    void post(const Value<std::int64_t>& value, std::int64_t new_value)
    {
        post_impl(value, new_value, get_fid(value));
    }

    void post(const Value<std::string>& value, std::string_view new_value)
    {
        post_impl(value, new_value, get_fid(value));
    }

    void post(
        const Value<std::vector<std::uint8_t>>& value,
        std::span<const std::uint8_t> new_value
    )
    {
        post_impl(value, new_value, get_fid(value));
    }
}
