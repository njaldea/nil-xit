#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <nil/xalt/fn_sign.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace nil::xit::tagged
{
    Value<bool>& add_value( //
        Frame& frame,
        std::string id,
        std::unique_ptr<IAccessor<bool>> accessor
    );

    Value<double>& add_value(
        Frame& frame,
        std::string id,
        std::unique_ptr<IAccessor<double>> accessor
    );

    Value<std::int64_t>& add_value(
        Frame& frame,
        std::string id,
        std::unique_ptr<IAccessor<std::int64_t>> accessor
    );

    Value<std::string>& add_value(
        Frame& frame,
        std::string id,
        std::unique_ptr<IAccessor<std::string>> accessor
    );

    Value<std::vector<std::uint8_t>>& add_value(
        Frame& frame,
        std::string id,
        std::unique_ptr<IAccessor<std::vector<std::uint8_t>>> accessor
    );

    template <typename T>
        requires(!is_built_in_value<typename T::type>)
    Value<T>& add_value(Frame& frame, std::string id, std::unique_ptr<T> accessor)
    {
        using inner_type = typename T::type;
        static_assert(has_codec<inner_type>, "requires buffer_type<T> serialize/deserialize");

        struct Accessor: IAccessor<std::vector<std::uint8_t>>
        {
            explicit Accessor(std::unique_ptr<T> init_accessor)
                : accessor(std::move(init_accessor))
            {
            }

            std::vector<std::uint8_t> get(std::string_view tag) const override
            {
                return buffer_type<inner_type>::serialize(accessor->get(tag));
            }

            void set(std::string_view tag, std::vector<std::uint8_t> value) override
            {
                accessor->set(
                    tag,
                    buffer_type<inner_type>::deserialize(value.data(), value.size())
                );
            }

            std::unique_ptr<T> accessor;
        };

        auto& obj = add_value( //
            frame,
            std::move(id),
            std::make_unique<Accessor>(std::move(accessor))
        );
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Value<T>&>(obj);
    }

    template <typename Getter>
        requires(std::is_invocable_v<Getter, std::string_view>)
    auto& add_value(Frame& frame, std::string id, Getter getter)
    {
        using type = typename nil::xalt::fn_sign<Getter>::return_type;

        struct Accessor: IAccessor<type>
        {
            explicit Accessor(Getter init_getter)
                : getter(std::move(init_getter))
            {
            }

            type get(std::string_view tag) const override
            {
                return getter(tag);
            }

            void set(std::string_view /* tag */, type /* value */) override
            {
            }

            Getter getter;
        };

        return add_value(
            frame,
            std::move(id),
            std::unique_ptr<IAccessor<type>>(new Accessor(std::move(getter)))
        );
    }

    template <typename Getter, typename Setter>
        requires(std::is_invocable_v<Getter, std::string_view>)
    auto& add_value(Frame& frame, std::string id, Getter getter, Setter setter)
    {
        using type = typename nil::xalt::fn_sign<Getter>::return_type;

        struct Accessor: IAccessor<type>
        {
            Accessor(Getter init_getter, Setter init_setter)
                : getter(std::move(init_getter))
                , setter(std::move(init_setter))
            {
            }

            type get(std::string_view tag) const override
            {
                return getter(tag);
            }

            void set(std::string_view tag, type value) override
            {
                setter(tag, std::move(value));
            }

            Getter getter;
            Setter setter;
        };

        auto& obj = add_value(
            frame,
            std::move(id),
            std::unique_ptr<IAccessor<type>>(new Accessor(std::move(getter), std::move(setter)))
        );
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Value<type>&>(obj);
    }
}
