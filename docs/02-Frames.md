# Frames

A Frame represents a C++-registered view, typically linked to a UI component.
Frames are used to scope data bindings, event signals, and UI logic.

There are two types of Frames: `Unique` / `Tagged`

## `Unique` and `Tagged` Frame

```cpp
auto server = nil::service::ws::server::create({.port = 1101});
auto core = nil::xit::create_core(server);

auto& unique_frame = add_unique_frame(
    core,
    "unique-frame-id",
    {
        .group="group1", // <-- group1 or group2 from set_groups
        .path="path/to/your/file.svelte"
    }
);

auto& tagged_frame = add_tagged_frame(
    core,
    "tagged-frame-id",
    {
        .group="group2", // <-- group1 or group2 from set_groups
        .path="path/to/your/file.svelte"
    }
);

set_groups(
    core,
    {
        {"group1", "paths..."},
        {"group2", "paths..."}
    }
);

```

## Values

You can add values to enable two-way data binding between the C++ application and the UI.

```cpp
auto& value = add_value(frame, "value_id", GETTER);
auto& value = add_value(frame, "value_id", GETTER, SETTER);
auto& value = add_value(frame, "value_id", ACCESSOR);
```

### Value Signatures

| Name     | Description                                 | Unique                  | Tagged                                    |
|----------|---------------------------------------------|-------------------------|-------------------------------------------|
| GETTER   | should return a value                       | `[](){ return VALUE; }` | `[](std::string_view){ return VALUE; }`   |
| SETTER   | will receive a value when changed in the UI | `[](type value){ ... }` | `[](std::string_view, type value){ ... }` |
| ACCESSOR | an object inheriting from nil::xit::TAG::IAccessor<T> that provides get/set methods | - | -                       |

Note: The only difference between `Unique`/`Tagged` Frame is the first std::string_view requirement of Tagged Frame.
This represents the tag ID and allows different data to be loaded for the same UI component.

### Posting Values from C++

From C++, you can update the frontend store by posting a new value.

```cpp
// Notify the UI about a value change for the unique frame.
post(value, new_value);

// Notify the UI about a value change for a specific tag.
post(value, "tag", new_value);
```

Note: Calling post updates the frontend store, but does not trigger the C++ setter. Setters are only invoked when the value changes from the UI.

### Accessing Values in the UI

Values are provided by the `xit` context. They are Svelte stores and are reactive.
Any changes to the store are sent to the C++ application. Any changes in the C++
application are synchronized with the stores.

The `values` accessor is a callable. The first argument is the value id. The second
argument is an optional codec object that has `encode` and `decode` methods. When a
codec is provided, it is used to convert between your custom type and a buffer.

```svelte
<script>
    import { xit } from "@nil-/xit";

    const { values } = xit();

    // No codec: use built-in buffer values.
    const store_buffer = values("id-buffer"); // Uint8Array or null

    // With codec: use your custom type.
    const point_codec = {
        encode(point) {
            const view = new DataView(new ArrayBuffer(16));
            view.setFloat64(0, point.x, true);
            view.setFloat64(8, point.y, true);
            return new Uint8Array(view.buffer);
        },
        decode(buffer) {
            const view = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);
            return { x: view.getFloat64(0, true), y: view.getFloat64(8, true) };
        },
    };

    const store_point = values("id-point", point_codec);

    // Use Svelte store API to access and update the data.
    $store_point = { x: 1.5, y: -2.0 };
</script>
```

## Signals

You can add signal listeners to handle events emitted from the frontend.

```cpp
add_signal(unique_frame, "id-void",    [](){});
add_signal(unique_frame, "id-bool",    [](bool){});
add_signal(unique_frame, "id-double",  [](double){});
add_signal(unique_frame, "id-number",  [](std::int64_t){});
add_signal(unique_frame, "id-string",  [](std::string_view){});
add_signal(unique_frame, "id-buffer",  [](std::span<const std::uint8_t>){});

add_signal(tagged_frame, "id-void",    [](std::string_view){});
add_signal(tagged_frame, "id-bool",    [](std::string_view, bool){});
add_signal(tagged_frame, "id-double",  [](std::string_view, double){});
add_signal(tagged_frame, "id-number",  [](std::string_view, std::int64_t){});
add_signal(tagged_frame, "id-string",  [](std::string_view, std::string_view){});
add_signal(tagged_frame, "id-buffer",  [](std::string_view, std::span<const std::uint8_t>){});
```

The only difference between unique/tagged frame is the first std::string_view requirement of tagged frame.

## Options

Options are key/value pairs attached to a frame and sent to the frontend to drive preprocessing.
The meaning of options is frontend-specific and can vary across UI layers.

```cpp
add_option(unique_frame, "theme", "dark");
add_option(tagged_frame, "layout", "grid");
```

### Svelte frontend behavior

In the current Svelte frontend, options are used for text replacement: keys are replaced with
their values in the Svelte files before rendering. Other frontends may interpret options
in different ways.

### Emitting Signals from the UI

Signals are provided by the `xit` context as a callable. The first argument is the signal id.
You can pass an optional encoder as the second argument. When an encoder is provided, you can
pass your custom type and it will be encoded automatically as a buffer. Without an encoder,
the callable accepts no argument or a buffer.

Signals allow the frontend to notify the backend of events (e.g. button clicks, data submissions).
They're defined in C++ and emitted from the GUI.

```svelte
<script>
    import { xit } from "@nil-/xit";

    const { signals } = xit();

    // No encoder: accepts no args or a buffer.
    const signal_none = signals("id-none");
    const signal_buffer = signals("id-buffer");

    // With encoder: accepts your custom type and encodes to a buffer.
    const signal_point = signals("id-point", (point) => {
        const view = new DataView(new ArrayBuffer(16));
        view.setFloat64(0, point.x, true);
        view.setFloat64(8, point.y, true);
        return new Uint8Array(view.buffer);
    });

    signal_none();
    signal_buffer(new Uint8Array([0, 1, 2]));
    signal_point({ x: 1.5, y: -2.0 });
</script>
```

