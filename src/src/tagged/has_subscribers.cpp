#include <nil/xit/tagged/has_subscribers.hpp>

#include "../structs.hpp"
#include "structs.hpp"

namespace nil::xit::tagged
{
    bool has_subscribers(const Frame& frame, std::string_view tag)
    {
        const auto _ = std::unique_lock(frame.core->mutex);
        if (const auto it = frame.subscribers.find(tag); it != frame.subscribers.end())
        {
            return !it->second.empty();
        }
        return false;
    }
}
