#include <nil/xit/methods/bind.hpp>

#include "../structs.hpp"

namespace nil::xit
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

    namespace impl
    {
        template <typename T, typename Getter, typename Setter>
        TaggedBinding<T>& bind(TaggedFrame& frame, std::string id, Getter getter, Setter setter)
        {
            using type = TaggedBinding<T>;
            auto binding = type{&frame, id, std::move(getter), std::move(setter)};
            return std::get<type>(frame.bindings.emplace(id, std::move(binding)).first->second);
        }
    }

    TaggedBinding<bool>& bind(
        TaggedFrame& frame,
        std::string id,
        std::function<bool(std::string_view)> getter,
        std::function<void(std::string_view, bool)> setter
    )
    {
        return impl::bind<bool>(frame, std::move(id), std::move(getter), std::move(setter));
    }

    TaggedBinding<double>& bind(
        TaggedFrame& frame,
        std::string id,
        std::function<double(std::string_view)> getter,
        std::function<void(std::string_view, double)> setter
    )
    {
        return impl::bind<double>(frame, std::move(id), std::move(getter), std::move(setter));
    }

    TaggedBinding<std::int64_t>& bind(
        TaggedFrame& frame,
        std::string id,
        std::function<std::int64_t(std::string_view)> getter,
        std::function<void(std::string_view, std::int64_t)> setter
    )
    {
        return impl::bind<std::int64_t>(frame, std::move(id), std::move(getter), std::move(setter));
    }

    TaggedBinding<std::string>& bind(
        TaggedFrame& frame,
        std::string id,
        std::function<std::string(std::string_view)> getter,
        std::function<void(std::string_view, std::string_view)> setter
    )
    {
        return impl::bind<std::string>(frame, std::move(id), std::move(getter), std::move(setter));
    }

    TaggedBinding<std::vector<std::uint8_t>>& bind(
        TaggedFrame& frame,
        std::string id,
        std::function<std::vector<std::uint8_t>(std::string_view)> getter,
        std::function<void(std::string_view, std::span<const std::uint8_t>)> setter
    )
    {
        return impl::bind<std::vector<std::uint8_t>>(
            frame,
            std::move(id),
            std::move(getter),
            std::move(setter)
        );
    }
}
