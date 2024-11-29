#include <nil/xit/unique/on_load.hpp>

#include "structs.hpp"

namespace nil::xit::unique
{
    void on_load(Frame& frame, std::function<void()> callback)
    {
        frame.on_load = std::move(callback);
    }
}
