#include <nil/xit/tagged/add_option.hpp>

#include "structs.hpp"

namespace nil::xit::tagged
{
    void add_option(Frame& frame, std::string key, std::string value)
    {
        frame.options.emplace(std::move(key), std::move(value));
    }
}
