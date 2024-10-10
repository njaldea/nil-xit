#include <nil/xit/tagged/add_value.hpp>

#include "structs.hpp"

namespace nil::xit::tagged::impl
{
    namespace impl
    {
        template <typename T, typename Getter, typename Setter>
        Value<T>& add_value(Frame& frame, std::string id, Getter getter, Setter setter)
        {
            using type = Value<T>;
            auto value = type{&frame, id, std::move(getter), std::move(setter)};
            return std::get<type>(frame.values.emplace(id, std::move(value)).first->second);
        }
    }

    Value<bool>& add_value(
        Frame& frame,
        std::string id,
        std::function<bool(std::string_view)> getter,
        std::function<void(std::string_view, bool)> setter
    )
    {
        return impl::add_value<bool>(frame, std::move(id), std::move(getter), std::move(setter));
    }

    Value<double>& add_value(
        Frame& frame,
        std::string id,
        std::function<double(std::string_view)> getter,
        std::function<void(std::string_view, double)> setter
    )
    {
        return impl::add_value<double>(frame, std::move(id), std::move(getter), std::move(setter));
    }

    Value<std::int64_t>& add_value(
        Frame& frame,
        std::string id,
        std::function<std::int64_t(std::string_view)> getter,
        std::function<void(std::string_view, std::int64_t)> setter
    )
    {
        return impl::add_value<std::int64_t>(
            frame,
            std::move(id),
            std::move(getter),
            std::move(setter)
        );
    }

    Value<std::string>& add_value(
        Frame& frame,
        std::string id,
        std::function<std::string(std::string_view)> getter,
        std::function<void(std::string_view, std::string_view)> setter
    )
    {
        return impl::add_value<std::string>(
            frame,
            std::move(id),
            std::move(getter),
            std::move(setter)
        );
    }

    Value<std::vector<std::uint8_t>>& add_value(
        Frame& frame,
        std::string id,
        std::function<std::vector<std::uint8_t>(std::string_view)> getter,
        std::function<void(std::string_view, std::span<const std::uint8_t>)> setter
    )
    {
        return impl::add_value<std::vector<std::uint8_t>>(
            frame,
            std::move(id),
            std::move(getter),
            std::move(setter)
        );
    }
}
