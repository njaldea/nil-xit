#pragma once

#include "structs.hpp"

#include "../buffer_type.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace nil::xit::tagged
{
    namespace impl
    {
        template <typename T>
        using return_t = decltype(std::declval<T>().operator()(std::declval<std::string_view>()));
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

    template <typename Getter>
        requires(!has_codec<impl::return_t<Getter>>)
    auto& add_value(Frame& frame, std::string id, Getter getter)
    {
        using type = decltype(std::declval<Getter>().operator()(std::declval<std::string_view>()));

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

            void set(std::string_view /* tag */, setter<type>::type /* value */) const override
            {
            }

            Getter getter;
        };

        return add_value(frame, std::move(id), std::make_unique<Accessor>(std::move(getter)));
    }

    template <typename Getter, typename Setter>
        requires(!has_codec<impl::return_t<Getter>>)
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

            type get(std::string_view tag) const override
            {
                return getter(tag);
            }

            void set(std::string_view tag, setter_t<type> value) const override
            {
                setter(tag, value);
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

    template <typename Getter>
        requires(has_codec<impl::return_t<Getter>>)
    auto& add_value(Frame& frame, std::string id, Getter getter)
    {
        using type = impl::return_t<Getter>;

        struct Accessor: IAccessor<std::vector<std::uint8_t>>
        {
            explicit Accessor(Getter init_getter)
                : getter(std::move(init_getter))
            {
            }

            std::vector<std::uint8_t> get(std::string_view tag) const override
            {
                return buffer_type<type>::serialize(getter(tag));
            }

            void set(std::string_view /* tag */, std::span<const std::uint8_t> /* value */)
                const override
            {
            }

            Getter getter;
        };

        auto& obj = add_value(frame, std::move(id), std::make_unique<Accessor>(std::move(getter)));
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Value<type>&>(obj);
    }

    template <typename Getter, typename Setter>
        requires(has_codec<impl::return_t<Getter>>)
    auto& add_value(Frame& frame, std::string id, Getter getter, Setter setter)
    {
        using type = impl::return_t<Getter>;

        struct Accessor: IAccessor<std::vector<std::uint8_t>>
        {
            Accessor(Getter init_getter, Setter init_setter)
                : getter(std::move(init_getter))
                , setter(std::move(init_setter))
            {
            }

            std::vector<std::uint8_t> get(std::string_view tag) const override
            {
                return buffer_type<type>::serialize(getter(tag));
            }

            void set(std::string_view tag, std::span<const std::uint8_t> value) const override
            {
                setter(tag, buffer_type<type>::deserialize(value.data(), value.size()));
            }

            Getter getter;
            Setter setter;
        };

        auto& obj = add_value(
            frame,
            std::move(id),
            std::make_unique<Accessor>(std::move(getter), std::move(setter))
        );
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Value<type>&>(obj);
    }
}
