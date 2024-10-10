#include <nil/xit/unique/add_value.hpp>

#include "structs.hpp"

namespace nil::xit::unique
{
    namespace impl
    {
        template <typename T, typename Callback>
        Value<T>& add_value(Frame& frame, std::string id, T new_value, Callback on_change)
        {
            using type = Value<T>;
            auto value = type{&frame, id, std::move(new_value), std::move(on_change)};
            return std::get<type>(frame.values.emplace(id, std::move(value)).first->second);
        }
    }

    Value<bool>& add_value(
        Frame& frame,
        std::string id,
        bool value,
        std::function<void(bool)> on_change
    )
    {
        return impl::add_value(frame, std::move(id), value, std::move(on_change));
    }

    Value<double>& add_value(
        Frame& frame,
        std::string id,
        double value,
        std::function<void(double)> on_change
    )
    {
        return impl::add_value(frame, std::move(id), value, std::move(on_change));
    }

    Value<std::int64_t>& add_value(
        Frame& frame,
        std::string id,
        std::int64_t value,
        std::function<void(std::int64_t)> on_change
    )
    {
        return impl::add_value(frame, std::move(id), value, std::move(on_change));
    }

    Value<std::string>& add_value(
        Frame& frame,
        std::string id,
        std::string value,
        std::function<void(std::string_view)> on_change
    )
    {
        return impl::add_value(frame, std::move(id), std::move(value), std::move(on_change));
    }

    Value<std::vector<std::uint8_t>>& add_value(
        Frame& frame,
        std::string id,
        std::vector<std::uint8_t> value,
        std::function<void(std::span<const std::uint8_t>)> on_change
    )
    {
        return impl::add_value(frame, std::move(id), std::move(value), std::move(on_change));
    }
}
