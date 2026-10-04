// Copyright (c) 2026, Neil Aldea <njaldea@gmail.com>
// SPDX-License-Identifier: BSL-1.0
// See LICENSE in the repository root and https://www.boost.org/LICENSE_1_0.txt.

#pragma once
#include "messages/message.fbs.h"

#include <nil/service/codec.hpp>

#include <flatbuffers/flatbuffer_builder.h>

namespace nil::service
{
    template <>
    struct codec<flatbuffers::FlatBufferBuilder>
    {
        static std::size_t size(const flatbuffers::FlatBufferBuilder& message)
        {
            return message.GetSize();
        }

        static std::size_t serialize(void* output, const flatbuffers::FlatBufferBuilder& message)
        {
            const auto size_used = size(message);
            std::copy(
                message.GetBufferPointer(),
                message.GetBufferPointer() + size_used,
                static_cast<std::uint8_t*>(output)
            );
            return size_used;
        }
    };

    template <>
    struct codec<nil::xit::fbs::MessageType>
    {
        using type = nil::xit::fbs::MessageType;

        static std::size_t size(const type& message)
        {
            return codec<std::int32_t>::size(std::int32_t(message));
        }

        static std::size_t serialize(void* output, const type& message)
        {
            return codec<std::int32_t>::serialize(output, std::int32_t(message));
        }

        static type deserialize(const void* data, std::uint64_t size)
        {
            return type(codec<std::int32_t>::deserialize(data, size));
        }
    };
}
