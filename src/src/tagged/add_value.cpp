#include <nil/xit/tagged/add_value.hpp>

#include "nil/xit/tagged/structs.hpp"
#include "structs.hpp"

namespace nil::xit::tagged
{
    template <typename T>
    using accessor_t = std::unique_ptr<IAccessor<T>>;

    template <typename T>
    Value<T>& add_value_impl(Frame& frame, std::string id, accessor_t<T> accessor)
    {
        return std::get<Value<T>>(
            frame.values.emplace(id, Value<T>{&frame, id, std::move(accessor)}).first->second
        );
    }

    Value<bool>& add_value(Frame& frame, std::string id, accessor_t<bool> accessor)
    {
        return add_value_impl(frame, std::move(id), std::move(accessor));
    }

    Value<double>& add_value(Frame& frame, std::string id, accessor_t<double> accessor)
    {
        return add_value_impl(frame, std::move(id), std::move(accessor));
    }

    Value<std::int64_t>& add_value(Frame& frame, std::string id, accessor_t<std::int64_t> accessor)
    {
        return add_value_impl(frame, std::move(id), std::move(accessor));
    }

    Value<std::string>& add_value(Frame& frame, std::string id, accessor_t<std::string> accessor)
    {
        return add_value_impl(frame, std::move(id), std::move(accessor));
    }

    Value<std::vector<std::uint8_t>>& add_value(
        Frame& frame,
        std::string id,
        accessor_t<std::vector<std::uint8_t>> accessor
    )
    {
        return add_value_impl(frame, std::move(id), std::move(accessor));
    }
}
