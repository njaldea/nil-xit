#pragma once

#include <nil/xit/buffer_type.hpp>

#include <nlohmann/json.hpp>

namespace nil::xit
{
    // this is necessary when publishing a custom data through the network going to the UI
    template <>
    struct buffer_type<nlohmann::json>
    {
        static nlohmann::json deserialize(const void* data, std::uint64_t size)
        {
            return nlohmann::json::parse(std::string_view(static_cast<const char*>(data), size));
        }

        static std::vector<std::uint8_t> serialize(const nlohmann::json& value)
        {
            auto s = value.dump();
            return {s.begin(), s.end()};
        }
    };
}

struct Ranges
{
    std::int64_t v1;
    std::int64_t v2;
    std::int64_t v3;

    // necessary for nil::gate edge dirty mechanism.
    bool operator==(const Ranges& o) const
    {
        return v1 == o.v1 && v2 == o.v2 && v3 == o.v3;
    }
};

inline nlohmann::json as_json(std::istream& iss)
{
    return nlohmann::json::parse(iss);
}

inline Ranges as_range(std::istream& iss)
{
    auto r = Ranges{};
    auto c = char{};
    iss >> c;
    iss >> r.v1;
    iss >> c;
    iss >> r.v2;
    iss >> c;
    iss >> r.v3;
    iss >> c;
    return r;
}

template <typename T = nlohmann::json>
auto from_json_ptr(const std::string& json_ptr)
{
    struct Accessor
    {
        T get(const nlohmann::json& data) const
        {
            return data[json_ptr];
        }

        void set(nlohmann::json& data, T new_data) const
        {
            data[json_ptr] = std::move(new_data);
        }

        nlohmann::json::json_pointer json_ptr;
    };

    return Accessor{nlohmann::json::json_pointer(json_ptr)};
}
