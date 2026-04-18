#include "utils.hpp"

#include "../codec.hpp" // IWYU pragma: keep
#include "../structs.hpp"
#include <ranges>

namespace nil::xit::tagged
{
    void post_impl(
        std::string_view tag,
        const Value<std::vector<std::uint8_t>>& value,
        std::vector<std::uint8_t> new_value,
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
            msg.value->value = new_value;

            flatbuffers::FlatBufferBuilder builder;
            builder.Finish(fbs::TaggedValueUpdate::Pack(builder, &msg));
            constexpr auto header = fbs::MessageType_Tagged_Value_Update;
            value.frame->core->msg_service->send(
                std::move(ids),
                nil::service::concat(header, builder)
            );
        }

        value.accessor->set(tag, std::move(new_value));
    }

    void subscribe(Frame& frame, std::string_view tag, const nil::service::ID& id)
    {
        auto it = frame.subscribers.find(tag);
        if (it == frame.subscribers.end())
        {
            it = frame.subscribers.emplace(tag, std::vector<nil::service::ID>()).first;
        }
        if (frame.on_sub)
        {
            frame.on_sub(tag, it->second.size() + 1);
        }
        it->second.push_back(id);
    }

    void unsubscribe(Frame& frame, std::string_view tag, const nil::service::ID& id)
    {
        const auto it = frame.subscribers.find(tag);
        if (it != frame.subscribers.end())
        {
            auto& subs = it->second;
            subs.erase(std::remove(subs.begin(), subs.end(), id), subs.end());
            if (subs.empty())
            {
                frame.subscribers.erase(it);
            }
            if (frame.on_sub)
            {
                frame.on_sub(tag, subs.size());
            }
        }
    }

    void unsubscribe(Frame& frame, const nil::service::ID& id)
    {
        for (auto it = frame.subscribers.begin(); it != frame.subscribers.end();)
        {
            auto& subs = it->second;
            subs.erase(std::remove(subs.begin(), subs.end(), id), subs.end());

            if (frame.on_sub)
            {
                frame.on_sub(it->first, it->second.size());
            }
            if (subs.empty())
            {
                it = frame.subscribers.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void load(const Frame& frame, std::string_view tag)
    {
        if (frame.on_load)
        {
            frame.on_load(tag);
        }
    }

    void value_set(
        Value<std::vector<std::uint8_t>>& value,
        const fbs::Value& msg,
        std::string_view tag,
        const nil::service::ID& id
    )
    {
        constexpr auto get_fid
            = [](auto& x_value, auto i_tag, auto& ex_tag) -> std::vector<nil::service::ID>
        {
            const auto not_ex_tag = [&ex_tag](const auto& sub_id) { return ex_tag != sub_id; };
            auto& subs = x_value.frame->subscribers;
            if (auto it = subs.find(i_tag); it != subs.end())
            {
                auto view = it->second | std::ranges::views::filter(not_ex_tag);
                return std::vector<nil::service::ID>(view.begin(), view.end());
            }
            return {};
        };

        post_impl(tag, value, {msg.value()->begin(), msg.value()->end()}, get_fid(value, tag, id));
    }

    void invoke(
        const Signal<std::span<const std::uint8_t>>& signal,
        const fbs::TaggedSignalNotify& msg,
        std::string_view tag
    )
    {
        if (signal.on_call)
        {
            const auto* ptr = msg.value();
            const auto span = std::span<const std::uint8_t>(
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
                reinterpret_cast<const std::uint8_t*>(ptr->data()),
                ptr->size()
            );
            signal.on_call(tag, span);
        }
    }
}
