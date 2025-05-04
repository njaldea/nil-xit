#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <nil/xalt/fn_sign.hpp>

#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

namespace nil::xit::tagged
{
    namespace impl
    {
        template <typename T>
        struct arg_type: arg_type<typename xalt::fn_sign<T>::arg_types>
        {
        };

        template <typename F, typename A>
            requires(std::is_same_v<std::remove_cvref_t<F>, std::string_view>)
        struct arg_type<nil::xalt::tlist_types<F, A>>
        {
            using type = std::remove_cvref_t<A>;
        };

        template <typename T>
        using arg_type_t = typename arg_type<T>::type;

        template <typename T>
        concept arg_none = std::invocable<T, std::string_view>;

        // clang-format off
        void add_signal(Frame& frame, std::string id, std::function<void(std::string_view)> callback);
        void add_signal(Frame& frame, std::string id, std::function<void(std::string_view, bool)> callback);
        void add_signal(Frame& frame, std::string id, std::function<void(std::string_view, double)> callback);
        void add_signal(Frame& frame, std::string id, std::function<void(std::string_view, std::int64_t)> callback);
        void add_signal(Frame& frame, std::string id, std::function<void(std::string_view, std::string_view)> callback);
        void add_signal(Frame& frame, std::string id, std::function<void(std::string_view, std::span<const std::uint8_t>)> callback);
        // clang-format on
    }

    template <impl::arg_none CB>
    void add_signal(Frame& frame, std::string id, CB callback)
    {
        impl::add_signal(
            frame,
            std::move(id),
            std::function<void(std::string_view)>(std::move(callback))
        );
    }

    template <typename CB>
        requires(!impl::arg_none<CB> && is_built_in<impl::arg_type_t<CB>>)
    void add_signal(Frame& frame, std::string id, CB callback)
    {
        using type = impl::arg_type_t<CB>;
        impl::add_signal(
            frame,
            std::move(id),
            std::function<void(std::string_view, type)>(std::move(callback))
        );
    }

    template <typename CB>
        requires(!impl::arg_none<CB> && !is_built_in<impl::arg_type_t<CB>>)
    void add_signal(Frame& frame, std::string id, CB callback)
    {
        using type = impl::arg_type_t<CB>;
        static_assert(has_deserialize<type>, "requires buffer_type<T>::deserialize");
        add_signal(
            frame,
            std::move(id),
            [callback
             = std::move(callback)](std::string_view tag, std::span<const std::uint8_t> data)
            { callback(tag, buffer_type<type>::deserialize(data.data(), data.size())); }
        );
    }
}
