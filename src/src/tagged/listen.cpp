#include <nil/xit/tagged/listen.hpp>

#include "../structs.hpp"

#include <functional>
#include <string>

namespace nil::xit::tagged::impl
{
    void listen(Frame& frame, std::string id, std::function<void(std::string_view)> callback)
    {
        auto listener = Listener<void>(std::move(callback));
        frame.listeners.emplace(std::move(id), std::move(listener));
    }

    void listen(Frame& frame, std::string id, std::function<void(std::string_view, bool)> callback)
    {
        auto listener = Listener<bool>(std::move(callback));
        frame.listeners.emplace(std::move(id), std::move(listener));
    }

    void listen(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, double)> callback
    )
    {
        auto listener = Listener<double>(std::move(callback));
        frame.listeners.emplace(std::move(id), std::move(listener));
    }

    void listen(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, std::int64_t)> callback
    )
    {
        auto listener = Listener<std::int64_t>(std::move(callback));
        frame.listeners.emplace(std::move(id), std::move(listener));
    }

    void listen(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, std::string_view)> callback
    )
    {
        auto listener = Listener<std::string_view>(std::move(callback));
        frame.listeners.emplace(std::move(id), std::move(listener));
    }

    void listen(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, std::span<const std::uint8_t>)> callback
    )
    {
        auto listener = Listener<std::span<const std::uint8_t>>(std::move(callback));
        frame.listeners.emplace(std::move(id), std::move(listener));
    }
}
