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
post("tag", value, new_value);
```

Note: Calling post updates the frontend store, but does not trigger the C++ setter. Setters are only invoked when the value changes from the UI.

### Accessing Values in the UI

Values are provided by the xit context. They are stores from Svelte which are reactive.
Any changes to the store, will be sent to the C++ application.
Any changes in the C++ application will be synchronized with the stores.

```svelte
<script>
    import { xit } from "@nil-/xit";

    const { values } = xit();

    // the 2nd argument is the default value and is going to be used
    // if the value is not available
    const store_boolean = values.boolean("id-bool", true);              // bool
    const store_double  = values.double("id-double", 1101.0);           // number
    const store_string  = values.string("id-string", "default_value");  // string
    const store_number  = values.number("id-number", 1101);             // number
    const store_buffer  = values.buffer("id-buffer", []);               // UInt8Array

    // Use Svelte store API to access and update the data.

    // this will set the data and C++ setter will be called
    $store_boolean = false;
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
add_signal(unique_frame, "id-buffer",  [](std::span<std::int8_t>){});

add_signal(tagged_frame, "id-void",    [](std::string_view){});
add_signal(tagged_frame, "id-bool",    [](std::string_view, bool){});
add_signal(tagged_frame, "id-double",  [](std::string_view, double){});
add_signal(tagged_frame, "id-number",  [](std::string_view, std::int64_t){});
add_signal(tagged_frame, "id-string",  [](std::string_view, std::string_view){});
add_signal(tagged_frame, "id-buffer",  [](std::string_view, std::span<std::int8_t>){});
```

The only difference between unique/tagged frame is the first std::string_view requirement of tagged frame.

### Emitting Signals from the UI

Signals are provided by the `xit` context. Simply call them like a function.

Signals allow the frontend to notify the backend of events (e.g. button clicks, data submissions). They're defined in C++ and emitted from the GUI.

```svelte
<script>
    import { xit } from "@nil-/xit";

    const { signals } = xit();

    const signal         = signals.none("id-none");     // void
    const signal_boolean = signals.boolean("id-bool");  // bool
    const signal_double  = signals.double("id-double"); // number
    const signal_number  = signals.number("id-number"); // number
    const signal_string  = signals.string("id-string"); // string
    const signal_buffer  = signals.buffer("id-buffer"); // UInt8Array

    // invoke with the right data.
    signal();
    signal_boolean(false);
</script>
```

