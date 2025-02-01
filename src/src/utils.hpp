#pragma once

#include "messages/message.fbs.h"

#include <type_traits>

namespace nil::xit::utils
{
    template <typename T>
    void unreachable();

    namespace transparent
    {
        struct Hash
        {
            using is_transparent = void;

            std::size_t operator()(std::string_view s) const
            {
                return std::hash<std::string_view>()(s);
            }
        };

        struct Equal
        {
            using is_transparent = void;

            bool operator()(std::string_view l, std::string_view r) const
            {
                return l == r;
            }
        };

        template <typename T>
        using hash_map = std::unordered_map<std::string, T, Hash, Equal>;
    }

    template <typename T>
    auto msg_set(const T& value, fbs::ValueT& msg, flatbuffers::FlatBufferBuilder& builder)
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            msg.value.Set(fbs::ValueBooleanT());
            msg.value.AsValueBoolean()->value = value;
            return fbs::ValueBoolean::Pack(builder, msg.value.AsValueBoolean());
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            msg.value.Set(fbs::ValueDoubleT());
            msg.value.AsValueDouble()->value = value;
            return fbs::ValueDouble::Pack(builder, msg.value.AsValueDouble());
        }
        else if constexpr (std::is_same_v<T, std::int64_t>)
        {
            msg.value.Set(fbs::ValueNumberT());
            msg.value.AsValueNumber()->value = value;
            return fbs::ValueNumber::Pack(builder, msg.value.AsValueNumber());
        }
        else if constexpr (std::is_same_v<T, std::string_view>)
        {
            msg.value.Set(fbs::ValueStringT());
            msg.value.AsValueString()->value = value;
            return fbs::ValueString::Pack(builder, msg.value.AsValueString());
        }
        else if constexpr (std::is_same_v<T, std::span<const std::uint8_t>>)
        {
            msg.value.Set(fbs::ValueBufferT());
            msg.value.AsValueBuffer()->value = {value.begin(), value.end()};
            return fbs::ValueBuffer::Pack(builder, msg.value.AsValueBuffer());
        }
        else
        {
            unreachable<T>();
        }
    }

    template <template <class> class Signal, typename T>
    void msg_set(const Signal<T>& /* signal */, fbs::SignalT& msg)
    {
        if constexpr (std::is_same_v<T, void>)
        {
            msg.type = fbs::SignalType_None;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            msg.type = fbs::SignalType_Boolean;
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            msg.type = fbs::SignalType_Double;
        }
        else if constexpr (std::is_same_v<T, std::int64_t>)
        {
            msg.type = fbs::SignalType_Number;
        }
        else if constexpr (std::is_same_v<T, std::string_view>)
        {
            msg.type = fbs::SignalType_String;
        }
        else if constexpr (std::is_same_v<T, std::span<const std::uint8_t>>)
        {
            msg.type = fbs::SignalType_Buffer;
        }
        else
        {
            unreachable<T>();
        }
    }
}
