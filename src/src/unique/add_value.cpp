#include <nil/xit/unique/add_value.hpp>

#include "structs.hpp"

namespace nil::xit::unique
{
    template <typename T>
    using accessor_t = std::unique_ptr<IAccessor<T>>;

    namespace impl
    {
        template <typename T>
        Value<T>& add_value(Frame& frame, std::string id, accessor_t<T> accessor)
        {
            using type = Value<T>;
            auto value = type{&frame, id, std::move(accessor)};
            return std::get<type>(frame.values.emplace(id, std::move(value)).first->second);
        }
    }

    Value<bool>& add_value(Frame& frame, std::string id, accessor_t<bool> accessor)
    {
        return impl::add_value<bool>(frame, std::move(id), std::move(accessor));
    }

    Value<double>& add_value(Frame& frame, std::string id, accessor_t<double> accessor)
    {
        return impl::add_value<double>(frame, std::move(id), std::move(accessor));
    }

    Value<std::int64_t>& add_value(Frame& frame, std::string id, accessor_t<std::int64_t> accessor)
    {
        return impl::add_value<std::int64_t>(frame, std::move(id), std::move(accessor));
    }

    Value<std::string>& add_value(Frame& frame, std::string id, accessor_t<std::string> accessor)
    {
        return impl::add_value<std::string>(frame, std::move(id), std::move(accessor));
    }

    Value<std::vector<std::uint8_t>>& add_value(
        Frame& frame,
        std::string id,
        accessor_t<std::vector<std::uint8_t>> accessor
    )
    {
        return impl::add_value<std::vector<std::uint8_t>>(
            frame,
            std::move(id),
            std::move(accessor)
        );
    }
}
