#pragma once

#include "../proto/message.pb.h"
#include "../utils.hpp"
#include "structs.hpp"

#include <nil/service/concat.hpp>

#include <algorithm>
#include <ranges>
#include <type_traits>

namespace nil::xit::unique
{
    template <typename T>
    void post_impl(
        const Value<T>& value,
        const setter_t<T>& v,
        const std::vector<nil::service::ID>& ids
    )
    {
        proto::ValueUpdate msg;
        msg.set_id(value.frame->id);

        auto* msg_value = msg.mutable_value();
        msg_value->set_id(value.id);
        nil::xit::utils::msg_set(std::move(v), *msg_value);

        constexpr auto header = proto::MessageType_ValueUpdate;
        send(*value.frame->core->service, ids, nil::service::concat(header, msg));
    }

    inline void subscribe(Frame& frame, std::string_view /* tag */, const nil::service::ID& id)
    {
        frame.subscribers.push_back(id);
        if (frame.on_sub)
        {
            frame.on_sub(frame.subscribers.size());
        }
    }

    inline void unsubscribe(Frame& frame, std::string_view /* tag */, const nil::service::ID& id)
    {
        auto& subs = frame.subscribers;
        subs.erase(std::remove(subs.begin(), subs.end(), id), subs.end());
        if (frame.on_sub)
        {
            frame.on_sub(frame.subscribers.size());
        }
    }

    inline void unsubscribe(Frame& frame, const nil::service::ID& id)
    {
        auto& subs = frame.subscribers;
        subs.erase(std::remove(subs.begin(), subs.end(), id), subs.end());
        if (frame.on_sub)
        {
            frame.on_sub(frame.subscribers.size());
        }
    }

    inline void load(const Frame& frame, std::string_view /* tag */)
    {
        if (frame.on_load)
        {
            frame.on_load();
        }
    }

    template <typename T>
    void msg_set(const Value<T>& value, proto::Value& msg, std::string_view /* tag */)
    {
        nil::xit::utils::msg_set(setter_t<T>(value.accessor->get()), msg);
    }

    template <typename T>
    void value_set( // NOLINT
        Value<T>& value,
        const proto::Value& msg,
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
            value.accessor->set(msg.value_boolean());
            if (const auto ids = get_fid(value, id); !ids.empty())
            {
                post_impl(value, msg.value_boolean(), ids);
            }
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            value.accessor->set(msg.value_double());
            if (auto ids = get_fid(value, id); !ids.empty())
            {
                post_impl(value, msg.value_double(), ids);
            }
        }
        else if constexpr (std::is_same_v<T, std::int64_t>)
        {
            value.accessor->set(msg.value_number());
            if (auto ids = get_fid(value, id); !ids.empty())
            {
                post_impl(value, msg.value_number(), ids);
            }
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            value.accessor->set(msg.value_string());
            if (auto ids = get_fid(value, id); !ids.empty())
            {
                post_impl(value, msg.value_string(), ids);
            }
        }
        else if constexpr (std::is_same_v<T, std::vector<std::uint8_t>>)
        {
            const auto& buffer = msg.value_buffer();
            const auto span = std::span<const std::uint8_t>(
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                reinterpret_cast<const std::uint8_t*>(buffer.data()),
                buffer.size()
            );
            value.accessor->set(span);
            if (auto ids = get_fid(value, id); !ids.empty())
            {
                post_impl(value, span, ids);
            }
        }
        else
        {
            nil::xit::utils::unreachable<T>();
        }
    }

    template <typename T>
    void invoke(const Signal<T>& signal, const proto::SignalNotify& msg, std::string_view /* tag */)
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
                signal.on_call(msg.arg_boolean());
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                signal.on_call(msg.arg_double());
            }
            else if constexpr (std::is_same_v<T, std::int64_t>)
            {
                signal.on_call(msg.arg_number());
            }
            else if constexpr (std::is_same_v<T, std::string_view>)
            {
                signal.on_call(msg.arg_string());
            }
            else if constexpr (std::is_same_v<T, std::span<const std::uint8_t>>)
            {
                const auto& buffer = msg.arg_buffer();
                const auto span = std::span<const std::uint8_t>(
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                    reinterpret_cast<const std::uint8_t*>(buffer.data()),
                    buffer.size()
                );
                signal.on_call(span);
            }
            else
            {
                nil::xit::utils::unreachable<T>();
            }
        }
    }
}
