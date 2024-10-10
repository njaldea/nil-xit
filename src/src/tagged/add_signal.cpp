#include <nil/xit/tagged/add_signal.hpp>

#include "structs.hpp"

#include <functional>
#include <string>

namespace nil::xit::tagged::impl
{
    void add_signal(Frame& frame, std::string id, std::function<void(std::string_view)> callback)
    {
        auto s = Signal<void>(std::move(callback));
        frame.signals.emplace(std::move(id), std::move(s));
    }

    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, bool)> callback
    )
    {
        auto s = Signal<bool>(std::move(callback));
        frame.signals.emplace(std::move(id), std::move(s));
    }

    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, double)> callback
    )
    {
        auto s = Signal<double>(std::move(callback));
        frame.signals.emplace(std::move(id), std::move(s));
    }

    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, std::int64_t)> callback
    )
    {
        auto s = Signal<std::int64_t>(std::move(callback));
        frame.signals.emplace(std::move(id), std::move(s));
    }

    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, std::string_view)> callback
    )
    {
        auto s = Signal<std::string_view>(std::move(callback));
        frame.signals.emplace(std::move(id), std::move(s));
    }

    void add_signal(
        Frame& frame,
        std::string id,
        std::function<void(std::string_view, std::span<const std::uint8_t>)> callback
    )
    {
        auto s = Signal<std::span<const std::uint8_t>>(std::move(callback));
        frame.signals.emplace(std::move(id), std::move(s));
    }
}
