// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#pragma once

#include <nil/xit/buffer_type.hpp>
#include <nil/xit/structs.hpp>
#include <nil/xit/unique/structs.hpp>

#include <string>

struct JSON
{
    std::string buffer;
};

template <>
struct nil::xit::buffer_type<JSON>
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

nil::xit::unique::Value<std::string>& add_base(nil::xit::Core& core);

void add_tagged(nil::xit::Core& core);

void add_group(nil::xit::Core& core);

void add_json_editor(nil::xit::Core& core);

void add_demo(nil::xit::Core& core);
