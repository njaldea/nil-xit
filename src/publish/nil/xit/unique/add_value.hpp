#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace nil::xit::unique
{
    namespace impl
    {
        Value<bool>& add_value(
            Frame& frame,
            std::string id,
            std::function<bool()> getter,
            std::function<void(bool)> setter
        );

        Value<double>& add_value(
            Frame& frame,
            std::string id,
            std::function<double()> getter,
            std::function<void(double)> setter
        );

        Value<std::int64_t>& add_value(
            Frame& frame,
            std::string id,
            std::function<std::int64_t()> getter,
            std::function<void(std::int64_t)> setter
        );

        Value<std::string>& add_value(
            Frame& frame,
            std::string id,
            std::function<std::string()> getter,
            std::function<void(std::string_view)> setter
        );

        Value<std::vector<std::uint8_t>>& add_value(
            Frame& frame,
            std::string id,
            std::function<std::vector<std::uint8_t>()> getter,
            std::function<void(std::span<const std::uint8_t>)> setter
        );

        template <typename T>
        concept has_codec = requires(T value) {
            { buffer_type<T>::serialize(value) } -> std::same_as<std::vector<std::uint8_t>>;
            { buffer_type<T>::deserialize(nullptr, 0) } -> std::same_as<T>;
        };

        template <typename T>
        using return_t = decltype(std::declval<T>()());
    }

    template <typename Getter>
        requires(!impl::has_codec<impl::return_t<Getter>>)
    auto& add_value(Frame& frame, std::string id, Getter getter)
    {
        using type = decltype(std::declval<Getter>()());
        return impl::add_value(
            frame,
            std::move(id),
            std::function<type()>(std::move(getter)),
            std::function<void(type)>()
        );
    }

    template <typename Getter, typename Setter>
        requires(!impl::has_codec<impl::return_t<Getter>>)
    auto& add_value(Frame& frame, std::string id, Getter getter, Setter setter)
    {
        using type = impl::return_t<Getter>;
        if constexpr (std::is_same_v<type, std::string>)
        {
            return impl::add_value(
                frame,
                std::move(id),
                std::function<std::string()>(std::move(getter)),
                std::function<void(std::string_view)>(std::move(setter))
            );
        }
        else if constexpr (std::is_same_v<type, std::vector<std::uint8_t>>)
        {
            return impl::add_value(
                frame,
                std::move(id),
                std::function<std::vector<std::uint8_t>()>(std::move(getter)),
                std::function<void(std::span<const std::uint8_t>)>(std::move(setter))
            );
        }
        else
        {
            return impl::add_value(
                frame,
                std::move(id),
                std::function<type()>(std::move(getter)),
                std::function<void(type)>(std::move(setter))
            );
        }
    }

    template <typename Getter>
        requires(impl::has_codec<impl::return_t<Getter>>)
    auto& add_value(Frame& frame, std::string id, Getter getter)
    {
        using type = impl::return_t<Getter>;
        auto& obj = impl::add_value(
            frame,
            std::move(id),
            std::function<std::vector<std::uint8_t>()>(
                [getter]() { return buffer_type<type>::serialize(getter()); }
            ),
            std::function<void(std::span<const std::uint8_t>)>()
        );
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Value<type>&>(obj);
    }

    template <typename Getter, typename Setter>
        requires(impl::has_codec<impl::return_t<Getter>>)
    auto& add_value(Frame& frame, std::string id, Getter getter, Setter setter)
    {
        using type = impl::return_t<Getter>;
        auto& obj = impl::add_value(
            frame,
            std::move(id),
            std::function<std::vector<std::uint8_t>()>(
                [getter]() { return buffer_type<type>::serialize(getter()); }
            ),
            std::function<void(std::span<const std::uint8_t>)>(
                [setter = std::move(setter)](std::span<const std::uint8_t> v)
                { setter(buffer_type<type>::deserialize(v.data(), v.size())); }
            )
        );
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Value<type>&>(obj);
    }
}
