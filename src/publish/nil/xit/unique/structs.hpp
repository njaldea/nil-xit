// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#pragma once

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
        virtual void set(T) = 0;
    };

    template <typename T>
    struct Value;
    struct Frame;
}
