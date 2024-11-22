#pragma once

#include "../setter_type.hpp"

namespace nil::xit::unique
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

        virtual T get() const = 0;
        virtual void set(setter_t<T>) const = 0;
    };
    template <typename T>
    struct Value;
    struct Frame;
}
