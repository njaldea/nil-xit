#include <nil/xit/methods/get.hpp>

#include "../structs/Core.hpp"

namespace nil::xit
{
    namespace impl
    {
        template <typename T, typename Callback>
        Binding<T>& bind(Frame& frame, std::string tag, T value, Callback on_change)
        {
            using type = Binding<T>;
            auto binding = type{&frame, tag, std::move(value), std::move(on_change)};
            return std::get<type>(frame.bindings.emplace(tag, std::move(binding)).first->second);
        }
    }

    Binding<std::int64_t>& bind(
        Frame& frame,
        std::string tag,
        std::int64_t value,
        std::function<void(std::int64_t)> on_change
    )
    {
        return impl::bind(frame, std::move(tag), value, std::move(on_change));
    }

    Binding<std::string>& bind(
        Frame& frame,
        std::string tag,
        std::string value,
        std::function<void(const std::string&)> on_change
    )
    {
        return impl::bind(frame, std::move(tag), std::move(value), std::move(on_change));
    }
}
