#include "utils.hpp"

namespace nil::xit::tagged
{
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
                if (frame.on_sub)
                {
                    frame.on_sub(tag, 0);
                }
            }
            else
            {
                if (frame.on_sub)
                {
                    frame.on_sub(tag, subs.size());
                }
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
}
