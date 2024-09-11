#include <nil/xit/methods/get.hpp>

#include "../structs.hpp"

namespace nil::xit
{
    std::int64_t get(const Binding<std::int64_t>& binding)
    {
        return binding.value;
    }

    std::string get(const Binding<std::string>& binding)
    {
        return binding.value;
    }

    std::vector<std::uint8_t> get(const Binding<std::vector<std::uint8_t>>& binding)
    {
        return binding.value;
    }
}
