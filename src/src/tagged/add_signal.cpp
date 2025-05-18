#include <nil/xit/tagged/add_signal.hpp>

#include "structs.hpp"

#include <functional>
#include <string>

namespace nil::xit::tagged::impl
{
    template <typename... T>
    void add_signal_impl(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, T...)> callback
    )
    {
        frame.signals.emplace(std::move(id), Signal<T...>(std::move(callback)));
    }

    void add_signal(Frame& frame, std::string id, std::function<void(std::string_view)> callback)
    {
        frame.signals.emplace(std::move(id), Signal<void>(std::move(callback)));
    }

    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, bool)> callback
    )
    {
        add_signal_impl(frame, std::move(id), std::move(callback));
    }

    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, double)> callback
    )
    {
        add_signal_impl(frame, std::move(id), std::move(callback));
    }

    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, std::int64_t)> callback
    )
    {
        add_signal_impl(frame, std::move(id), std::move(callback));
    }

    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, std::string_view)> callback
    )
    {
        add_signal_impl(frame, std::move(id), std::move(callback));
    }

    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, std::span<const std::uint8_t>)> callback
    )
    {
        add_signal_impl(frame, std::move(id), std::move(callback));
    }
}
