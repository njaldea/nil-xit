#pragma once

#include "../messages/message.fbs.h"
#include "structs.hpp"

#include <nil/service/concat.hpp>
#include <nil/xalt/errors.hpp>

#include <flatbuffers/flatbuffer_builder.h>

namespace nil::xit::tagged
{
    void post_impl(
        std::string_view tag,
        const Value<std::vector<std::uint8_t>>& value,
        std::vector<std::uint8_t> new_value,
        std::vector<nil::service::ID> ids
    );

    void subscribe(Frame& frame, std::string_view tag, const nil::service::ID& id);
    void unsubscribe(Frame& frame, std::string_view tag, const nil::service::ID& id);
    void unsubscribe(Frame& frame, const nil::service::ID& id);
    void load(const Frame& frame, std::string_view tag);

    void value_set(
        Value<std::vector<std::uint8_t>>& value,
        const fbs::Value& msg,
        std::string_view tag,
        const nil::service::ID& id
    );

    void invoke(
        const Signal<std::span<const std::uint8_t>>& signal,
        const fbs::TaggedSignalNotify& msg,
        std::string_view tag
    );
}
