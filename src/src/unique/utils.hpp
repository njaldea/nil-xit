#pragma once

#include "../messages/message.fbs.h"
#include "structs.hpp"

#include <nil/service/concat.hpp>
#include <nil/xalt/errors.hpp>

#include <flatbuffers/flatbuffer_builder.h>

namespace nil::xit::unique
{
    void post_impl(
        const Value<std::vector<std::uint8_t>>& value,
        std::vector<std::uint8_t> new_value,
        std::vector<nil::service::ID> ids
    );

    void subscribe(Frame& frame, const nil::service::ID& id);
    void unsubscribe(Frame& frame, const nil::service::ID& id);
    void load(const Frame& frame);

    void value_set(
        Value<std::vector<std::uint8_t>>& value,
        const fbs::Value& msg,
        const nil::service::ID& id
    );

    void invoke(
        const Signal<std::span<const std::uint8_t>>& signal,
        const fbs::UniqueSignalNotify& msg
    );
}
