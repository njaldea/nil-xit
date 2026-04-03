#pragma once

#include <nil/xit.h>
#include <nil/xit/unique/structs.hpp>

#include <cstdint>
#include <optional>
#include <vector>

namespace nil::xit::c
{
    class UniqueAccessor final: public nil::xit::unique::IAccessor<std::vector<std::uint8_t>>
    {
    public:
        explicit UniqueAccessor(nil_xit_unique_value_accessor init_accessor)
            : accessor(init_accessor)
        {
        }

        UniqueAccessor(const UniqueAccessor&) = delete;
        UniqueAccessor& operator=(const UniqueAccessor&) = delete;
        UniqueAccessor(UniqueAccessor&&) = delete;
        UniqueAccessor& operator=(UniqueAccessor&&) = delete;

        ~UniqueAccessor() noexcept override
        {
            if (accessor.cleanup != nullptr)
            {
                accessor.cleanup(accessor.ctx);
            }
        }

        std::vector<std::uint8_t> get() const override
        {
            if (accessor.encode == nullptr || accessor.encode_size == nullptr)
            {
                return {};
            }

            if (!cached_value.has_value())
            {
                const auto size = accessor.encode_size(accessor.ctx);
                std::vector<std::uint8_t> result(size);
                accessor.encode(accessor.ctx, result.data());
                cached_value = std::move(result);
            }
            return cached_value.value();
        }

        void set(std::vector<std::uint8_t> value) override
        {
            if (accessor.decode == nullptr)
            {
                return;
            }

            cached_value = std::move(value);
            accessor.decode(accessor.ctx, cached_value->data(), cached_value->size());
        }

    private:
        nil_xit_unique_value_accessor accessor;
        mutable std::optional<std::vector<std::uint8_t>> cached_value;
    };
}
