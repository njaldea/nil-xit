#pragma once

#include "../messages/message.fbs.h"
#include "../utils.hpp"
#include "structs.hpp"

#include <nil/service/concat.hpp>
#include <nil/xalt/errors.hpp>

#include <flatbuffers/flatbuffer_builder.h>

#include <ranges>
#include <type_traits>

namespace nil::xit::tagged
{
    template <typename T, typename U>
        requires(std::is_same_v<T, std::remove_cvref_t<U>>)
    void post_impl(
        std::string_view tag,
        const Value<T>& value,
        U&& new_value,
        std::vector<nil::service::ID> ids
    )
    {
        if (!ids.empty())
        {
            fbs::TaggedValueUpdateT msg;
            msg.id = value.frame->id;
            msg.tag = tag;
            msg.value = std::make_unique<fbs::ValueT>();
            msg.value->id = value.id;
            flatbuffers::FlatBufferBuilder tmp_builder;
            nil::xit::utils::msg_set(std::forward<U>(new_value), *msg.value, tmp_builder);

            flatbuffers::FlatBufferBuilder builder;
            builder.Finish(fbs::TaggedValueUpdate::Pack(builder, &msg));
            constexpr auto header = fbs::MessageType_Tagged_Value_Update;
            send(
                *value.frame->core->service,
                std::move(ids),
                nil::service::concat(header, builder)
            );
        }
    }

    void subscribe(Frame& frame, std::string_view tag, const nil::service::ID& id);
    void unsubscribe(Frame& frame, std::string_view tag, const nil::service::ID& id);
    void unsubscribe(Frame& frame, const nil::service::ID& id);
    void load(const Frame& frame, std::string_view tag);

    template <typename T>
    auto msg_set(
        const Value<T>& value,
        fbs::ValueT& msg,
        flatbuffers::FlatBufferBuilder& builder,
        std::string_view tag
    )
    {
        msg.id = value.id;
        return nil::xit::utils::msg_set(value.accessor->get(tag), msg, builder);
    }

    template <typename T>
    void value_set(
        Value<T>& value,
        const fbs::Value& msg,
        std::string_view tag,
        const nil::service::ID& id
    )
    {
        auto data = [](const fbs::Value& m)
        {
            if constexpr (std::is_same_v<T, bool>)
            {
                return m.value_as_ValueBoolean()->value();
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                return m.value_as_ValueDouble()->value();
            }
            else if constexpr (std::is_same_v<T, std::int64_t>)
            {
                return m.value_as_ValueNumber()->value();
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                return m.value_as_ValueString()->value()->str();
            }
            else if constexpr (std::is_same_v<T, std::vector<std::uint8_t>>)
            {
                const auto* v = m.value_as_ValueBuffer()->value();
                return std::vector<std::uint8_t>(v->begin(), v->end());
            }
            else
            {
                nil::xalt::undefined<T>();
            }
        }(msg);

        constexpr auto get_fid
            = [](auto& x_value, auto i_tag, auto& ex_tag) -> std::vector<nil::service::ID>
        {
            const auto not_ex_tag = [&ex_tag](const auto& sub_id) { return ex_tag != sub_id; };
            auto _ = std::lock_guard(x_value.frame->core->mutex);
            auto& subs = x_value.frame->subscribers;
            if (auto it = subs.find(i_tag); it != subs.end())
            {
                auto view = it->second | std::ranges::views::filter(not_ex_tag);
                return std::vector<nil::service::ID>(view.begin(), view.end());
            }
            return {};
        };
        if (auto ids = get_fid(value, tag, id); !ids.empty())
        {
            post_impl(tag, value, data, std::move(ids));
        }
        value.accessor->set(tag, std::move(data));
    }

    template <typename T>
    void invoke(const Signal<T>& signal, const fbs::TaggedSignalNotify& msg, std::string_view tag)
    {
        if (signal.on_call)
        {
            if constexpr (std::is_same_v<T, void>)
            {
                signal.on_call(tag);
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                signal.on_call(tag, msg.value_as_ValueBoolean()->value());
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                signal.on_call(tag, msg.value_as_ValueDouble()->value());
            }
            else if constexpr (std::is_same_v<T, std::int64_t>)
            {
                signal.on_call(tag, msg.value_as_ValueNumber()->value());
            }
            else if constexpr (std::is_same_v<T, std::string_view>)
            {
                signal.on_call(tag, msg.value_as_ValueString()->value()->string_view());
            }
            else if constexpr (std::is_same_v<T, std::span<const std::uint8_t>>)
            {
                const auto* ptr = msg.value_as_ValueBuffer()->value();
                const auto span = std::span<const std::uint8_t>(
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                    reinterpret_cast<const std::uint8_t*>(ptr->data()),
                    ptr->size()
                );
                signal.on_call(tag, span);
            }
            else
            {
                nil::xalt::undefined<T>();
            }
        }
    }
}
