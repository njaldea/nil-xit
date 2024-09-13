#pragma once

#include "../structs.hpp"

#include <functional>
#include <span>
#include <string>
#include <string_view>

namespace nil::xit
{
    namespace impl
    {
        template <typename T>
        struct first_arg: first_arg<decltype(&T::operator())>
        {
        };

        template <typename C, typename A>
        struct first_arg<void (C::*)(A) const>: first_arg<void (*)(A)>
        {
        };

        template <typename C, typename A>
        struct first_arg<void (C::*)(A)>: first_arg<void (*)(A)>
        {
        };

        template <typename A>
        struct first_arg<void (*)(A)>
        {
            using type = std::decay_t<A>;
        };

        template <typename T>
        concept no_arg = std::invocable<T>;

        template <typename T>
        concept has_deserialize = requires(T arg) {
            {
                buffer_type<typename impl::first_arg<T>::type>::deserialize(nullptr, 0)
            } -> std::same_as<typename impl::first_arg<T>::type>;
        };

        void listen(Frame& frame, std::string tag, std::function<void()> callback);
        void listen(Frame& frame, std::string tag, std::function<void(bool)> callback);
        void listen(Frame& frame, std::string tag, std::function<void(double)> callback);
        void listen(Frame& frame, std::string tag, std::function<void(std::int64_t)> callback);
        void listen(Frame& frame, std::string tag, std::function<void(std::string_view)> callback);
        void listen(
            Frame& frame,
            std::string tag,
            std::function<void(std::span<const std::uint8_t>)> callback
        );
    }

    template <impl::no_arg CB>
    void listen(Frame& frame, std::string tag, CB callback)
    {
        impl::listen(frame, std::move(tag), std::function<void()>(std::move(callback)));
    }

    template <typename CB>
        requires(!impl::no_arg<CB>) && (!impl::has_deserialize<CB>)
    void listen(Frame& frame, std::string tag, CB callback)
    {
        using type = typename impl::first_arg<CB>::type;
        impl::listen(frame, std::move(tag), std::function<void(type)>(std::move(callback)));
    }

    template <typename CB>
        requires(!impl::no_arg<CB>) && impl::has_deserialize<CB>
    void listen(Frame& frame, std::string tag, CB callback)
    {
        using type = typename impl::first_arg<CB>::type;
        listen(
            frame,
            std::move(tag),
            [callback = std::move(callback)](std::span<const std::uint8_t> data)
            { callback(buffer_type<type>::deserialize(data.data(), data.size())); }
        );
    }
}
