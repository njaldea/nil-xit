#include <nil/xit/unique/add_signal.hpp>

#include "structs.hpp"

#include <functional>
#include <string>

namespace nil::xit::unique::impl
{
    template <typename... T>
    void add_signal_impl(Frame& frame, std::string id, std::function<void(T...)> callback)
    {
        frame.signals.emplace(std::move(id), Signal<T...>(std::move(callback)));
    }

    void add_signal(Frame& frame, std::string id, std::function<void()> callback)
    {
        frame.signals.emplace(std::move(id), Signal<void>(std::move(callback)));
    }

    void add_signal(Frame& frame, std::string id, std::function<void(bool)> callback)
    {
        add_signal_impl(frame, std::move(id), std::move(callback));
    }

    void add_signal(Frame& frame, std::string id, std::function<void(double)> callback)
    {
        add_signal_impl(frame, std::move(id), std::move(callback));
    }

    void add_signal(Frame& frame, std::string id, std::function<void(std::int64_t)> callback)
    {
        add_signal_impl(frame, std::move(id), std::move(callback));
    }

    void add_signal(Frame& frame, std::string id, std::function<void(std::string_view)> callback)
    {
        add_signal_impl(frame, std::move(id), std::move(callback));
    }

    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::span<const std::uint8_t>)> callback
    )
    {
        add_signal_impl<>(frame, std::move(id), std::move(callback));
    }
}
