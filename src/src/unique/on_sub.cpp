#include <nil/xit/unique/on_sub.hpp>

#include "structs.hpp"

namespace nil::xit::unique
{
    void on_sub(Frame& frame, std::function<void(std::size_t)> callback)
    {
        frame.on_sub = std::move(callback);
    }
}
