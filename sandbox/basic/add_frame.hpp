#pragma once

#include <nil/xit/buffer_type.hpp>
#include <nil/xit/structs.hpp>
#include <nil/xit/unique/structs.hpp>

#include <string>

struct JSON
{
    std::string buffer;
};

namespace nil::xit
{
    template <>
    struct buffer_type<JSON>
    {
        static JSON deserialize(const void* data, std::uint64_t size)
        {
            return JSON{std::string(static_cast<const char*>(data), size)};
        }

        static std::vector<std::uint8_t> serialize(const JSON& value)
        {
            return {value.buffer.begin(), value.buffer.end()};
        }
    };
}

nil::xit::unique::Value<std::string>& add_base(nil::xit::Core& core);

void add_tagged(nil::xit::Core& core);

void add_group(nil::xit::Core& core);

void add_json_editor(nil::xit::Core& core);

void add_demo(nil::xit::Core& core);
