#include <nil/xit/unique/has_subscribers.hpp>

#include "../structs.hpp"
#include "structs.hpp"

namespace nil::xit::unique
{
    bool has_subscribers(const Frame& frame)
    {
        const auto _ = std::lock_guard(frame.core->mutex);
        return !frame.subscribers.empty();
    }
}
