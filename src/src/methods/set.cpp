#include <nil/xit/methods/post.hpp>
#include <nil/xit/structs.hpp>

#include "../structs/Binding.hpp"

namespace nil::xit
{
    void set(Binding<std::int64_t>& binding, std::int64_t value)
    {
        binding.value = value;
        post(binding, binding.value);
    }

    void set(Binding<std::string>& binding, std::string value)
    {
        binding.value = std::move(value);
        post(binding, binding.value);
    }
}
