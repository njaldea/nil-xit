#pragma once

#include "../proto/message.pb.h"
#include "../utils.hpp"
#include "structs.hpp"

#include <nil/service/concat.hpp>

#include <algorithm>
#include <type_traits>

namespace nil::xit::unique
{
    template <typename T>
    void post_impl(const Value<T>& value, T v, const std::vector<nil::service::ID>& ids)
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
    }

    inline void unsubscribe(Frame& frame, std::string_view /* tag */, const nil::service::ID& id)
    {
        auto& subs = frame.subscribers;
        subs.erase(std::remove(subs.begin(), subs.end(), id), subs.end());
    }

    inline void unsubscribe(Frame& frame, const nil::service::ID& id)
    {
        auto& subs = frame.subscribers;
        subs.erase(std::remove(subs.begin(), subs.end(), id), subs.end());
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
        nil::xit::utils::msg_set(value.value, msg);
    }

    template <typename T>
    void value_set(
        Value<T>& value,
        const proto::Value& msg,
        std::string_view /* tag */,
        const nil::service::ID& id
    )
    {
        constexpr auto apply = [](Value<T>& v, const proto::Value& m)
        {
            constexpr auto set = [](T& l, auto&& r)
            {
                if (l != r)
                {
                    l = std::forward<decltype(r)>(r);
                    return true;
                }
                return false;
            };
            if constexpr (std::is_same_v<T, bool>)
            {
                return set(v.value, m.value_boolean());
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                return set(v.value, m.value_double());
            }
            else if constexpr (std::is_same_v<T, std::int64_t>)
            {
                return set(v.value, m.value_number());
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                return set(v.value, m.value_string());
            }
            else if constexpr (std::is_same_v<T, std::vector<std::uint8_t>>)
            {
                const auto& mm = m.value_buffer();
                const auto& vv = v.value;
                if (vv.size() != mm.size() || 0 != std::memcmp(vv.data(), mm.data(), vv.size()))
                {
                    v.value = {m.value_buffer().begin(), m.value_buffer().end()};
                    return true;
                }
                return false;
            }
            else
            {
                nil::xit::utils::unreachable<T>();
            }
        };

        if (apply(value, msg))
        {
            if (value.on_change)
            {
                value.on_change(value.value);
            }
            auto ids = [&]()
            {
                auto ret = [&]()
                {
                    std::vector<nil::service::ID> r;
                    auto _ = std::lock_guard(value.frame->core->mutex);
                    std::copy_if(
                        value.frame->subscribers.begin(),
                        value.frame->subscribers.end(),
                        std::back_inserter(r),
                        [&](const nil::service::ID& t) { return t != id; }
                    );
                    return r;
                }();
                return ret;
            }();
            if (!ids.empty())
            {
                post_impl(value, value.value, ids);
            }
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
