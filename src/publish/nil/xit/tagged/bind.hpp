#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace nil::xit::tagged
{
    namespace impl
    {
        Binding<bool>& bind(
            Frame& frame,
            std::string id,
            std::function<bool(std::string_view)> getter,
            std::function<void(std::string_view, bool)> setter
        );

        Binding<double>& bind(
            Frame& frame,
            std::string id,
            std::function<double(std::string_view)> getter,
            std::function<void(std::string_view, double)> setter
        );

        Binding<std::int64_t>& bind(
            Frame& frame,
            std::string id,
            std::function<std::int64_t(std::string_view)> getter,
            std::function<void(std::string_view, std::int64_t)> setter
        );

        Binding<std::string>& bind(
            Frame& frame,
            std::string id,
            std::function<std::string(std::string_view)> getter,
            std::function<void(std::string_view, std::string_view)> setter
        );

        Binding<std::vector<std::uint8_t>>& bind(
            Frame& frame,
            std::string id,
            std::function<std::vector<std::uint8_t>(std::string_view)> getter,
            std::function<void(std::string_view, std::span<const std::uint8_t>)> setter
        );

        template <typename T>
        concept has_codec = requires(T value) {
            { buffer_type<T>::serialize(value) } -> std::same_as<std::vector<std::uint8_t>>;
            { buffer_type<T>::deserialize(nullptr, 0) } -> std::same_as<T>;
        };

        template <typename T>
        using return_t = decltype(std::declval<T>().operator()(std::declval<std::string_view>()));
    }

    template <typename Getter>
        requires(!impl::has_codec<impl::return_t<Getter>>)
    auto& bind(Frame& frame, std::string id, Getter getter)
    {
        using type = decltype(std::declval<Getter>().operator()(std::declval<std::string_view>()));
        return impl::bind(
            frame,
            std::move(id),
            std::function<type(std::string_view)>(std::move(getter)),
            std::function<void(std::string_view, type)>()
        );
    }

    template <typename Getter, typename Setter>
        requires(!impl::has_codec<impl::return_t<Getter>>)
    auto& bind(Frame& frame, std::string id, Getter getter, Setter setter)
    {
        using type = impl::return_t<Getter>;
        if constexpr (std::is_same_v<type, std::string>)
        {
            return impl::bind(
                frame,
                std::move(id),
                std::function<std::string(std::string_view)>(std::move(getter)),
                std::function<void(std::string_view, std::string_view)>(std::move(setter))
            );
        }
        else if constexpr (std::is_same_v<type, std::vector<std::uint8_t>>)
        {
            return impl::bind(
                frame,
                std::move(id),
                std::function<std::vector<std::uint8_t>(std::string_view)>(std::move(getter)),
                std::function<void(std::string_view, std::span<const std::uint8_t>)>(
                    std::move(setter)
                )
            );
        }
        else
        {
            return impl::bind(
                frame,
                std::move(id),
                std::function<type(std::string_view)>(std::move(getter)),
                std::function<void(std::string_view, type)>(std::move(setter))
            );
        }
    }

    template <typename Getter>
        requires(impl::has_codec<impl::return_t<Getter>>)
    auto& bind(Frame& frame, std::string id, Getter getter)
    {
        using type = impl::return_t<Getter>;
        auto& obj = impl::bind(
            frame,
            std::move(id),
            std::function<std::vector<std::uint8_t>(std::string_view)>(
                [getter](std::string_view i) { return buffer_type<type>::serialize(getter(i)); }
            ),
            std::function<void(std::string_view, std::span<const std::uint8_t>)>()
        );
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Binding<type>&>(obj);
    }

    template <typename Getter, typename Setter>
        requires(impl::has_codec<impl::return_t<Getter>>)
    auto&& bind(Frame& frame, std::string id, Getter getter, Setter setter)
    {
        using type = impl::return_t<Getter>;
        auto& obj = impl::bind(
            frame,
            std::move(id),
            std::function<std::vector<std::uint8_t>(std::string_view)>(
                [getter](std::string_view i) { return buffer_type<type>::serialize(getter(i)); }
            ),
            std::function<void(std::string_view, std::span<const std::uint8_t>)>(
                [setter = std::move(setter)](std::string_view i, std::span<const std::uint8_t> v)
                {
                    if (setter)
                    {
                        setter(i, buffer_type<type>::deserialize(v.data(), v.size()));
                    }
                }
            )
        );
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Binding<type>&>(obj);
    }
}
