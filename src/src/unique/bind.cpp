#include <nil/xit/unique/bind.hpp>

#include "structs.hpp"

namespace nil::xit::unique
{
    namespace impl
    {
        template <typename T, typename Callback>
        Binding<T>& bind(Frame& frame, std::string id, T value, Callback on_change)
        {
            using type = Binding<T>;
            auto binding = type{&frame, id, std::move(value), std::move(on_change)};
            return std::get<type>(frame.bindings.emplace(id, std::move(binding)).first->second);
        }
    }

    Binding<bool>& bind(
        Frame& frame,
        std::string id,
        bool value,
        std::function<void(bool)> on_change
    )
    {
        return impl::bind(frame, std::move(id), value, std::move(on_change));
    }

    Binding<double>& bind(
        Frame& frame,
        std::string id,
        double value,
        std::function<void(double)> on_change
    )
    {
        return impl::bind(frame, std::move(id), value, std::move(on_change));
    }

    Binding<std::int64_t>& bind(
        Frame& frame,
        std::string id,
        std::int64_t value,
        std::function<void(std::int64_t)> on_change
    )
    {
        return impl::bind(frame, std::move(id), value, std::move(on_change));
    }

    Binding<std::string>& bind(
        Frame& frame,
        std::string id,
        std::string value,
        std::function<void(std::string_view)> on_change
    )
    {
        return impl::bind(frame, std::move(id), std::move(value), std::move(on_change));
    }

    Binding<std::vector<std::uint8_t>>& bind(
        Frame& frame,
        std::string id,
        std::vector<std::uint8_t> value,
        std::function<void(std::span<const std::uint8_t>)> on_change
    )
    {
        return impl::bind(frame, std::move(id), std::move(value), std::move(on_change));
    }
}
