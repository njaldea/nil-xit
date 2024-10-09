#include <nil/xit/tagged/bind.hpp>

#include "structs.hpp"

namespace nil::xit::tagged::impl
{
    namespace impl
    {
        template <typename T, typename Getter, typename Setter>
        Binding<T>& bind(Frame& frame, std::string id, Getter getter, Setter setter)
        {
            using type = Binding<T>;
            auto binding = type{&frame, id, std::move(getter), std::move(setter)};
            return std::get<type>(frame.bindings.emplace(id, std::move(binding)).first->second);
        }
    }

    Binding<bool>& bind(
        Frame& frame,
        std::string id,
        std::function<bool(std::string_view)> getter,
        std::function<void(std::string_view, bool)> setter
    )
    {
        return impl::bind<bool>(frame, std::move(id), std::move(getter), std::move(setter));
    }

    Binding<double>& bind(
        Frame& frame,
        std::string id,
        std::function<double(std::string_view)> getter,
        std::function<void(std::string_view, double)> setter
    )
    {
        return impl::bind<double>(frame, std::move(id), std::move(getter), std::move(setter));
    }

    Binding<std::int64_t>& bind(
        Frame& frame,
        std::string id,
        std::function<std::int64_t(std::string_view)> getter,
        std::function<void(std::string_view, std::int64_t)> setter
    )
    {
        return impl::bind<std::int64_t>(frame, std::move(id), std::move(getter), std::move(setter));
    }

    Binding<std::string>& bind(
        Frame& frame,
        std::string id,
        std::function<std::string(std::string_view)> getter,
        std::function<void(std::string_view, std::string_view)> setter
    )
    {
        return impl::bind<std::string>(frame, std::move(id), std::move(getter), std::move(setter));
    }

    Binding<std::vector<std::uint8_t>>& bind(
        Frame& frame,
        std::string id,
        std::function<std::vector<std::uint8_t>(std::string_view)> getter,
        std::function<void(std::string_view, std::span<const std::uint8_t>)> setter
    )
    {
        return impl::bind<std::vector<std::uint8_t>>(
            frame,
            std::move(id),
            std::move(getter),
            std::move(setter)
        );
    }
}
