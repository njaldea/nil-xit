#include <nil/xit/tagged/on_load.hpp>

#include "structs.hpp"

namespace nil::xit::tagged
{
    void on_load(Frame& frame, std::function<void(std::string_view)> callback)
    {
        frame.on_load = std::move(callback);
    }
}
