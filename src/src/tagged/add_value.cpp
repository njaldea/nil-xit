#include <nil/xit/tagged/add_value.hpp>

#include "structs.hpp"

namespace nil::xit::tagged::impl
{
    Value<std::vector<std::uint8_t>>& add_value(
        Frame& frame,
        std::string id,
        std::unique_ptr<IAccessor<std::vector<std::uint8_t>>> accessor
    )
    {
        return frame.values
            .emplace(id, Value<std::vector<std::uint8_t>>{&frame, id, std::move(accessor)})
            .first->second;
    }
}
