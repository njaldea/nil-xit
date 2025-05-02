#pragma once

#include "../messages/message.fbs.h"
#include "../utils.hpp"
#include "structs.hpp"

#include <nil/service/concat.hpp>
#include <nil/xalt/errors.hpp>

#include <flatbuffers/flatbuffer_builder.h>

#include <ranges>

namespace nil::xit::unique
{
    template <typename T>
    void post_impl(
        const Value<T>& value,
        const setter_t<T>& new_value,
        const std::vector<nil::service::ID>& ids
    )
    {
        fbs::UniqueValueUpdateT msg;
        msg.id = value.frame->id;
        msg.value = std::make_unique<fbs::ValueT>();
        msg.value->id = value.id;
        flatbuffers::FlatBufferBuilder tmp_builder;
        nil::xit::utils::msg_set(new_value, *msg.value, tmp_builder);

        flatbuffers::FlatBufferBuilder builder;
        builder.Finish(fbs::UniqueValueUpdate::Pack(builder, &msg));
        constexpr auto header = fbs::MessageType_Unique_Value_Update;
        send(*value.frame->core->service, ids, nil::service::concat(header, builder));
    }

    void subscribe(Frame& frame, std::string_view /* tag */, const nil::service::ID& id);
    void unsubscribe(Frame& frame, std::string_view /* tag */, const nil::service::ID& id);
    void unsubscribe(Frame& frame, const nil::service::ID& id);
    void load(const Frame& frame, std::string_view /* tag */);

    template <typename T>
    auto msg_set(
        const Value<T>& value,
        fbs::ValueT& msg,
        flatbuffers::FlatBufferBuilder& builder,
        std::string_view /* tag */
    )
    {
        msg.id = value.id;
        return nil::xit::utils::msg_set(setter_t<T>(value.accessor->get()), msg, builder);
    }

    template <typename T>
    void value_set( // NOLINT
        Value<T>& value,
        const fbs::Value& msg,
        std::string_view /* tag */,
        const nil::service::ID& id
    )
    {
        constexpr auto get_fid = [](auto& x_value, auto& ex_tag)
        {
            const auto not_ex_tag = [ex_tag](const auto& sub_id) { return ex_tag != sub_id; };
            auto _ = std::lock_guard(x_value.frame->core->mutex);
            auto& subscribers = x_value.frame->subscribers;
            auto view = subscribers | std::ranges::views::filter(not_ex_tag);
            return std::vector<nil::service::ID>(view.begin(), view.end());
        };
        if constexpr (std::is_same_v<T, bool>)
        {
            value.accessor->set(msg.value_as_ValueBoolean()->value());
            if (const auto ids = get_fid(value, id); !ids.empty())
            {
                post_impl(value, msg.value_as_ValueBoolean()->value(), ids);
            }
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            value.accessor->set(msg.value_as_ValueDouble()->value());
            if (auto ids = get_fid(value, id); !ids.empty())
            {
                post_impl(value, msg.value_as_ValueDouble()->value(), ids);
            }
        }
        else if constexpr (std::is_same_v<T, std::int64_t>)
        {
            value.accessor->set(msg.value_as_ValueNumber()->value());
            if (auto ids = get_fid(value, id); !ids.empty())
            {
                post_impl(value, msg.value_as_ValueNumber()->value(), ids);
            }
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            value.accessor->set(msg.value_as_ValueString()->value()->str());
            if (auto ids = get_fid(value, id); !ids.empty())
            {
                post_impl(value, msg.value_as_ValueString()->value()->str(), ids);
            }
        }
        else if constexpr (std::is_same_v<T, std::vector<std::uint8_t>>)
        {
            const auto& buffer = msg.value_as_ValueBuffer()->value();
            const auto span = std::span<const std::uint8_t>(
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                reinterpret_cast<const std::uint8_t*>(buffer->data()),
                buffer->size()
            );
            value.accessor->set(span);
            if (auto ids = get_fid(value, id); !ids.empty())
            {
                post_impl(value, span, ids);
            }
        }
        else
        {
            nil::xalt::undefined<T>();
        }
    }

    template <typename T>
    void invoke(
        const Signal<T>& signal,
        const fbs::UniqueSignalNotify& msg,
        std::string_view /* tag */
    )
    {
        if (signal.on_call)
        {
            if constexpr (std::is_same_v<T, void>)
            {
                (void)msg;
                signal.on_call();
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                signal.on_call(msg.value_as_ValueBoolean()->value());
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                signal.on_call(msg.value_as_ValueDouble()->value());
            }
            else if constexpr (std::is_same_v<T, std::int64_t>)
            {
                signal.on_call(msg.value_as_ValueNumber()->value());
            }
            else if constexpr (std::is_same_v<T, std::string_view>)
            {
                signal.on_call(msg.value_as_ValueString()->value()->string_view());
            }
            else if constexpr (std::is_same_v<T, std::span<const std::uint8_t>>)
            {
                const auto* ptr = msg.value_as_ValueBuffer()->value();
                const auto span = std::span<const std::uint8_t>(
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                    reinterpret_cast<const std::uint8_t*>(ptr->data()),
                    ptr->size()
                );
                signal.on_call(span);
            }
            else
            {
                nil::xalt::undefined<T>();
            }
        }
    }
}
