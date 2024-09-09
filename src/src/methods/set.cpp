#include <nil/xit/methods/post.hpp>
#include <nil/xit/structs.hpp>

#include "../structs.hpp"

namespace nil::xit
{
    namespace impl
    {
        template <typename T>
        void set(Binding<T>& binding, T value)
        {
            if (binding.value != value)
            {
                binding.value = value;
                post(binding, binding.value);
            }
        }
    }

    void set(Binding<std::int64_t>& binding, std::int64_t value)
    {
        impl::set(binding, value);
    }

    void set(Binding<std::string>& binding, std::string value)
    {
        impl::set(binding, std::move(value));
    }
}
