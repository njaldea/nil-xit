#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace nil::xit::unique
{
    namespace impl
    {
        template <typename T>
        using return_t = decltype(std::declval<T>()());
        template <typename Getter>
        concept is_valid_getter = requires(Getter getter) {
            { getter() };
        };
        template <typename Getter, typename Setter>
        concept is_valid_setter = requires(Getter getter, Setter setter) {
            { setter(std::declval<setter_t<decltype(getter())>>()) } -> std::same_as<void>;
        };
    }

    Value<bool>& add_value(
        Frame& frame,
        std::string id,
        std::unique_ptr<IAccessor<bool>> accessor //
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
        requires(!is_built_in<T>)
    Value<T>& add_value(Frame& frame, std::string id, std::unique_ptr<IAccessor<T>> accessor)
    {
        static_assert(has_codec<T>, "requires buffer_type<T> serialize/deserialize");

        struct Accessor: IAccessor<std::vector<std::uint8_t>>
        {
            explicit Accessor(std::unique_ptr<IAccessor<T>> init_accessor)
                : accessor(std::move(init_accessor))
            {
            }

            std::vector<std::uint8_t> get() const override
            {
                return buffer_type<T>::serialize(accessor->get());
            }

            void set(std::span<const std::uint8_t> value) const override
            {
                accessor->set(buffer_type<T>::deserialize(value.data(), value.size()));
            }

            std::unique_ptr<IAccessor<T>> accessor;
        };

        auto& obj
            = add_value(frame, std::move(id), std::make_unique<Accessor>(std::move(accessor)));
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Value<T>&>(obj);
    }

    template <impl::is_valid_getter Getter>
    auto& add_value(Frame& frame, std::string id, Getter getter)
    {
        using type = decltype(std::declval<Getter>()());

        struct Accessor: IAccessor<type>
        {
            explicit Accessor(Getter init_getter)
                : getter(std::move(init_getter))
            {
            }

            type get() const override
            {
                return getter();
            }

            void set(setter<type>::type /* value */) const override
            {
            }

            Getter getter;
        };

        return add_value(frame, std::move(id), std::make_unique<Accessor>(std::move(getter)));
    }

    template <typename Getter, typename Setter>
        requires(impl::is_valid_setter<Getter, Setter>)
    auto& add_value(Frame& frame, std::string id, Getter getter, Setter setter)
    {
        using type = impl::return_t<Getter>;

        struct Accessor: IAccessor<type>
        {
            Accessor(Getter init_getter, Setter init_setter)
                : getter(std::move(init_getter))
                , setter(std::move(init_setter))
            {
            }

            type get() const override
            {
                return getter();
            }

            void set(setter_t<type> value) const override
            {
                setter(value);
            }

            Getter getter;
            Setter setter;
        };

        return add_value(
            frame,
            std::move(id),
            std::make_unique<Accessor>(std::move(getter), std::move(setter))
        );
    }

    template <typename T>
        requires(std::is_base_of_v<IAccessor<typename T::type>, T>)
    Value<typename T::type>& add_value(Frame& frame, std::string id, std::unique_ptr<T> accessor)
    {
        return add_value(
            frame,
            std::move(id),
            std::unique_ptr<IAccessor<typename T::type>>(std::move(accessor))
        );
    }
}
