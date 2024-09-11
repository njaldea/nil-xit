#pragma once

#include <nil/service/IService.hpp>

#include <memory>

namespace nil::xit
{
    template <typename T>
    struct Binding;

    struct Frame;
    struct Core;

    struct C
    {
        std::unique_ptr<Core, void (*)(Core*)> ptr;

        operator Core&() const // NOLINT
        {
            return *ptr;
        }
    };

    C make_core(nil::service::IService& service);

    template <typename T>
    struct buffered_traits
    {
        static T deserialize(const void* data, std::uint64_t size);
        static std::vector<std::uint8_t> serialize(const T& value);
    };
}
