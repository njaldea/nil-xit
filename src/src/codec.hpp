#include "messages/message.fbs.h"

#include <nil/service/codec.hpp>

#include <flatbuffers/flatbuffer_builder.h>

namespace nil::service
{
    template <>
    struct codec<flatbuffers::FlatBufferBuilder>
    {
        using type = nil::xit::fbs::MessageType;

        static std::vector<std::uint8_t> serialize(const flatbuffers::FlatBufferBuilder& message)
        {
            return {message.GetBufferPointer(), message.GetBufferPointer() + message.GetSize()};
        }
    };

    template <>
    struct codec<nil::xit::fbs::MessageType>
    {
        using type = nil::xit::fbs::MessageType;

        static std::vector<std::uint8_t> serialize(const type& message)
        {
            return codec<std::int32_t>::serialize(std::int32_t(message));
        }

        static type deserialize(const void* data, std::uint64_t& size)
        {
            return type(codec<std::int32_t>::deserialize(data, size));
        }
    };
}
