#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <nil/xalt/errors.hpp>
#include <nil/xalt/fn_sign.hpp>

#include <functional>
#include <span>
#include <string>
#include <string_view>

namespace nil::xit::unique
{
    namespace impl
    {
        template <typename T>
        using arg_type_t
            = std::remove_cvref_t<typename xalt::fn_sign<T>::arg_types::template at<0>>;

        template <typename T>
        concept arg_none = xalt::fn_sign<T>::arg_types::size == 0;

        // clang-format off
        void add_signal(Frame& frame, std::string id, std::function<void()> callback);
        void add_signal(Frame& frame, std::string id, std::function<void(bool)> callback);
        void add_signal(Frame& frame, std::string id, std::function<void(double)> callback);
        void add_signal(Frame& frame, std::string id, std::function<void(std::int64_t)> callback);
        void add_signal(Frame& frame, std::string id, std::function<void(std::string_view)> callback);
        void add_signal(Frame& frame, std::string id, std::function<void(std::span<const std::uint8_t>)> callback);
        // clang-format on
    }

    template <impl::arg_none CB>
    void add_signal(Frame& frame, std::string id, CB callback)
    {
        impl::add_signal(frame, std::move(id), std::function<void()>(std::move(callback)));
    }

    template <typename CB>
        requires(!impl::arg_none<CB> && is_built_in_signal<impl::arg_type_t<CB>>)
    void add_signal(Frame& frame, std::string id, CB callback)
    {
        impl::add_signal(
            frame,
            std::move(id),
            std::function<void(impl::arg_type_t<CB>)>(std::move(callback))
        );
    }

    template <typename CB>
        requires(!impl::arg_none<CB> && !is_built_in_signal<impl::arg_type_t<CB>>)
    void add_signal(Frame& frame, std::string id, CB callback)
    {
        using type = impl::arg_type_t<CB>;
        static_assert(has_deserialize<type>, "requires buffer_type<T>::deserialize");
        add_signal(
            frame,
            std::move(id),
            [callback = std::move(callback)] //
            (std::span<const std::uint8_t> data)
            { callback(buffer_type<type>::deserialize(data.data(), data.size())); }
        );
    }
}
