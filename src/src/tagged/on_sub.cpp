#include <nil/xit/tagged/on_sub.hpp>

#include "structs.hpp"

namespace nil::xit::tagged
{
    void on_sub(Frame& frame, std::function<void(std::string_view, std::size_t)> callback)
    {
        frame.on_sub = std::move(callback);
    }
}
