#pragma once

#include "../structs.hpp"

#include <functional>
#include <span>
#include <string>

namespace nil::xit
{
    void listen(Frame& frame, std::string tag, std::function<void()> callback);
    void listen(Frame& frame, std::string tag, std::function<void(std::int64_t)> callback);
    void listen(Frame& frame, std::string tag, std::function<void(const std::string&)> callback);
    void listen(
        Frame& frame,
        std::string tag,
        std::function<void(std::span<const std::uint8_t>)> callback
    );

    template <typename T>
    void listen(Frame& frame, std::string tag, std::function<void(const T&)> callback)
    {
        listen(
            frame,
            std::move(tag),
            [callback = std::move(callback)](const std::span<const std::uint8_t>& data)
            { callback(buffer_type<T>::deserialize(data.data(), data.size())); }
        );
    }
}
