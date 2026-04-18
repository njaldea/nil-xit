#include "utils.hpp"

#include "../codec.hpp" // IWYU pragma: keep
#include "../structs.hpp"

#include <ranges>

namespace nil::xit::unique
{
    void post_impl(
        const Value<std::vector<std::uint8_t>>& value,
        std::vector<std::uint8_t> new_value,
        std::vector<nil::service::ID> ids
    )
    {
        if (!ids.empty())
        {
            fbs::UniqueValueUpdateT msg;
            msg.id = value.frame->id;
            msg.value = std::make_unique<fbs::ValueT>();
            msg.value->id = value.id;
            msg.value->value = new_value;

            flatbuffers::FlatBufferBuilder builder;
            builder.Finish(fbs::UniqueValueUpdate::Pack(builder, &msg));
            constexpr auto header = fbs::MessageType_Unique_Value_Update;
            value.frame->core->msg_service->send(
                std::move(ids),
                nil::service::concat(header, builder)
            );
        }

        value.accessor->set(std::move(new_value));
    }

    void subscribe(Frame& frame, const nil::service::ID& id)
    {
        frame.subscribers.push_back(id);
        if (frame.on_sub)
        {
            frame.on_sub(frame.subscribers.size());
        }
    }

    void unsubscribe(Frame& frame, const nil::service::ID& id)
    {
        auto& subs = frame.subscribers;
        subs.erase(std::remove(subs.begin(), subs.end(), id), subs.end());
        if (frame.on_sub)
        {
            frame.on_sub(frame.subscribers.size());
        }
    }

    void load(const Frame& frame)
    {
        if (frame.on_load)
        {
            frame.on_load();
        }
    }

    void value_set(
        Value<std::vector<std::uint8_t>>& value,
        const fbs::Value& msg,
        const nil::service::ID& id
    )
    {
        constexpr auto get_fid = [](auto& x_value, auto& ex_tag)
        {
            const auto not_ex_tag = [ex_tag](const auto& sub_id) { return ex_tag != sub_id; };
            auto view = x_value.frame->subscribers | std::ranges::views::filter(not_ex_tag);
            return std::vector<nil::service::ID>(view.begin(), view.end());
        };

        post_impl(value, {msg.value()->begin(), msg.value()->end()}, get_fid(value, id));
    }

    void invoke(
        const Signal<std::span<const std::uint8_t>>& signal,
        const fbs::UniqueSignalNotify& msg
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
            signal.on_call(span);
        }
    }
}
