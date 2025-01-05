#include "utils.hpp"

#include "../structs.hpp"

namespace nil::xit::unique
{
    void subscribe(Frame& frame, std::string_view /* tag */, const nil::service::ID& id)
    {
        auto _ = std::lock_guard(frame.core->mutex);
        frame.subscribers.push_back(id);
        if (frame.on_sub)
        {
            frame.on_sub(frame.subscribers.size());
        }
    }

    void unsubscribe(Frame& frame, std::string_view /* tag */, const nil::service::ID& id)
    {
        auto _ = std::lock_guard(frame.core->mutex);
        auto& subs = frame.subscribers;
        subs.erase(std::remove(subs.begin(), subs.end(), id), subs.end());
        if (frame.on_sub)
        {
            frame.on_sub(frame.subscribers.size());
        }
    }

    void unsubscribe(Frame& frame, const nil::service::ID& id)
    {
        auto _ = std::lock_guard(frame.core->mutex);
        auto& subs = frame.subscribers;
        subs.erase(std::remove(subs.begin(), subs.end(), id), subs.end());
        if (frame.on_sub)
        {
            frame.on_sub(frame.subscribers.size());
        }
    }

    void load(const Frame& frame, std::string_view /* tag */)
    {
        if (frame.on_load)
        {
            frame.on_load();
        }
    }
}
