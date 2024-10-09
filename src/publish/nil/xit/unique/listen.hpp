#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <functional>
#include <span>
#include <string>
#include <string_view>

namespace nil::xit::unique
{
    namespace impl
    {
        template <typename T>
        struct first_arg: first_arg<decltype(&T::operator())>
        {
        };

        template <typename C, typename... A>
        struct first_arg<void (C::*)(A...) const>: first_arg<void (*)(A...)>
        {
        };

        template <typename C, typename... A>
        struct first_arg<void (C::*)(A...)>: first_arg<void (*)(A...)>
        {
        };

        template <typename A>
        struct first_arg<void (*)(A)>
        {
            using type = std::decay_t<A>;
        };

        template <typename A>
        struct first_arg<void (*)(std::string_view, A)>
        {
            using type = std::decay_t<A>;
        };

        template <typename T>
        using first_arg_t = typename first_arg<T>::type;

        template <typename T>
        concept arg_none = std::invocable<T>;

        template <typename T>
        concept has_deserialize = requires(T arg) {
            {
                buffer_type<first_arg_t<T>>::deserialize(nullptr, 0)
            } -> std::same_as<first_arg_t<T>>;
        };

        // clang-format off
        void listen(Frame& frame, std::string id, std::function<void()> callback);
        void listen(Frame& frame, std::string id, std::function<void(bool)> callback);
        void listen(Frame& frame, std::string id, std::function<void(double)> callback);
        void listen(Frame& frame, std::string id, std::function<void(std::int64_t)> callback);
        void listen(Frame& frame, std::string id, std::function<void(std::string_view)> callback);
        void listen(Frame& frame, std::string id, std::function<void(std::span<const std::uint8_t>)> callback);
        // clang-format on
    }

    template <impl::arg_none CB>
    void listen(Frame& frame, std::string id, CB callback)
    {
        impl::listen(frame, std::move(id), std::function<void()>(std::move(callback)));
    }

    template <typename CB>
        requires(!impl::arg_none<CB> && !impl::has_deserialize<CB>)
    void listen(Frame& frame, std::string id, CB callback)
    {
        using type = impl::first_arg_t<CB>;
        impl::listen(frame, std::move(id), std::function<void(type)>(std::move(callback)));
    }

    template <typename CB>
        requires(!impl::arg_none<CB> && impl::has_deserialize<CB>)
    void listen(Frame& frame, std::string id, CB callback)
    {
        using type = impl::first_arg_t<CB>;
        listen(
            frame,
            std::move(id),
            [callback = std::move(callback)](std::span<const std::uint8_t> data)
            { callback(buffer_type<type>::deserialize(data.data(), data.size())); }
        );
    }
}
