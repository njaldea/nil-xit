#pragma once

#include <string_view>

namespace nil::xit::tagged
{
    template <typename T>
    struct IAccessor
    {
        using type = T;

        IAccessor() = default;
        virtual ~IAccessor() = default;
        IAccessor(IAccessor&&) = delete;
        IAccessor(const IAccessor&) = delete;
        IAccessor& operator=(IAccessor&&) = delete;
        IAccessor& operator=(const IAccessor&) = delete;

        virtual T get(std::string_view) const = 0;
        virtual void set(std::string_view, T) = 0;
    };

    template <typename T>
    struct Value;
    struct Frame;
}
