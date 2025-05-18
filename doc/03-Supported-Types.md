# Supported Types

Supported types are the following:

- boolean
    - `bool`
- double
    - `double`
- number
    - `std::int64_t`
- string
    - `std::string` for values
    - `std::string_view` for signals
- buffer
    - `std::vector<uint8_t>` for values
    - `std::span<const uint8_t>` for signals

## Custom Types

To support custom types, they need to be serializable to buffer types.

The example below creates a JSON struct with a stringified buffer payload.

```cpp
struct JSON
{
    std::string buffer;
};

template <>
struct nil::xit::buffer_type<JSON>
{
    static JSON deserialize(const void* data, std::uint64_t size)
    {
        return {.buffer = {static_cast<const char*>(data), size}};
    }

    static std::vector<std::uint8_t> serialize(const JSON& value)
    {
        return {value.buffer.begin(), value.buffer.end()};
    }
};
```

### Example

```cpp
auto& unique_value = add_value(
    unique_frame,
    "id-value",
    JSON(),
    [](const JSON& value){ /* ... */ }
);
post(unique_value, JSON());

add_signal(
    unique_frame,
    "id-signal",
    [](const JSON& value){ /* ... */ }
);

auto& tagged_value = add_value(
    tagged_frame,
    "id-value",
    [](std::string_view tag){ return JSON(); },
    [](std::string_view tag, const JSON& value){ /* ... */ }
);
post("tag", tagged_value, JSON());

add_signal(
    tagged_frame,
    "id-signal",
    [](std::string_view, tag, const JSON& value){ /* ... */ }
);
```

### Custom Types in UI(svelte)

`values` and `signals` provide a `json` type which expects to receive and encoder/decoder methods.

```svelte
<script>
    import {
        xit,
        // A basic json_string codec is provided by `@nil-/xit` library.
        // This simply uses `JSON.stringify` and `JSON.parse`.
        json_string
    } from "@nil-/xit";

    const { signals } = xit();

    const value_json = values.json("id-value", {} /* default value */, json_string);
    const signal_json = signals.json("id-signal", json_string.encode);

    // ...

    $value_json = { hello: "world" };
    signal_json({ hello: "world" });
</script>
```