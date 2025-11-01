# nil/xit

Bridge C++ backends and any UI/client with a tiny, explicit protocol. Define frames, values, and signals in C++; keep your client simple and reactive. The client can be web, desktop, or any runtime that speaks the protocol.

- Frames group values and signals displayed by a UI component
- Two flavors: unique (one instance) and tagged (keyed by a tag)
- Values expose get/set and can be posted from C++ to all subscribers
- Signals represent UI → C++ events (optionally with payload)

See also:
- [How It Works](./doc/01-How-It-Works.md)
- [Frames](./doc/02-Frames.md)
- [Supported Types](./doc/03-Supported-Types.md)
- [Number Issues](./doc/04-Number-Issues.md)

Protocol note
- Messages are defined in FlatBuffers (see `src/src/messages/message.fbs`).
- Transport is provided by nil/service (commonly WebSocket), but any transport works if it delivers/receives the same bytes.
- The Svelte example below is just one client; any client that implements the schema can participate.

## Core

Create a core on top of a messaging service and (optionally) map UI asset groups.

Key helpers (headers under `src/publish/nil/xit/`):
- `make_core(...)` – construct the core
- `add_unique_frame(core, id [, path])`
- `add_tagged_frame(core, id [, path])`
- `set_groups(core, { {group, path}, ... })`
- `set_cache_directory(core, path)`

Example bootstrap (abbreviated):

```cpp
auto server = nil::service::http::server::create({/*...*/});
nil::xit::setup_server(server, {"node_modules/@nil-/xit/assets"});
auto ws = use_ws(server, "/ws");
auto core = nil::xit::make_core(ws);

auto& uframe = add_unique_frame(core, "base", "$base/gui/Base.svelte");
auto& tframe = add_tagged_frame(core, "tagged", "$base/gui/Tagged.svelte");

start(server); // service thread handles UI messages
```

## Values – unique

Add a value with a getter (and optional setter). Supported payloads: `bool`, `std::int64_t`, `double`, `std::string`, `std::vector<std::uint8_t>`. Custom types are supported via `buffer_type<T>`.

Headers: `unique/add_value.hpp`, `unique/post.hpp`

```cpp
int counter = 0;

add_value(
    uframe,
    "counter",
    []() -> std::int64_t { return counter; },
    [](std::int64_t v) { counter = static_cast<int>(v); }
);

// Push a new value to all subscribers (service thread context)
nil::xit::unique::post(uframe, "counter", std::int64_t{42});
```

Notes
- Without a setter, the value is read-only from UI.
- Posting re-broadcasts to subscribers of the frame.

## Values – tagged

Tagged values receive a `tag` in get/set/post.

Headers: `tagged/add_value.hpp`, `tagged/post.hpp`

```cpp
add_value(
    tframe,
    "score",
    [](std::string_view tag) -> std::int64_t { /* lookup by tag */ return 100; },
    [](std::string_view tag, std::int64_t v) { /* store by tag */ }
);

nil::xit::tagged::post("player-42", tframe, "score", std::int64_t{7});
```

## Signals

Register callbacks invoked when the UI emits events. Signals may be `void`, `bool`, `int64_t`, `double`, `string_view`, or `span<const uint8_t>` (custom types via `buffer_type<T>`).

Headers: `unique/add_signal.hpp`, `tagged/add_signal.hpp`

Unique example:

```cpp
add_signal(uframe, "reset", []() {
    // handle UI-triggered reset
});

add_signal(uframe, "log", [](std::string_view msg) {
    // handle message
});
```

Tagged example:

```cpp
add_signal(tframe, "notify", [](std::string_view tag, std::string_view msg) {
    // per-tag notification
});
```

## Lifecycle hooks

Per-frame hooks to react to client activity.

Headers: `unique/on_load.hpp`, `unique/on_sub.hpp`, `tagged/on_load.hpp`, `tagged/on_sub.hpp`

```cpp
nil::xit::unique::on_load(uframe, [] { /* a client loaded the frame */ });
nil::xit::unique::on_sub(uframe, [](std::size_t n) { /* subscriber count changed */ });
```

## Subscriptions

Subscriptions are tracked per frame. Updates posted to a value are broadcast to all subscribers of that frame (for tagged frames: to subscribers of the tag).

## Threading model

- Before `start(...)`: configure frames/values/signals from a single thread.
- After `start(...)`: mutations that affect subscribers must run on the service thread. Use the provided `post(...)` functions for broadcasting from C++.

## UI usage (example)

Using the sibling JS package from Svelte:

```svelte
<script>
    import { xit } from "@nil-/xit";
    const { values, signals } = xit();

    const int_value = values.number('tagged_value', 1101);
    const string_signal = signals.string('tagged_signal');

    const click = () => {
        int_value.update(v => v + 1);
        string_signal(`${$int_value} published`);
    };
</script>

<button onclick={click}>tagged {$int_value}</button>
```

## Docs

- [How It Works](./doc/01-How-It-Works.md)
- [Frames](./doc/02-Frames.md)
- [Supported Types](./doc/03-Supported-Types.md)
- [Number Issues](./doc/04-Number-Issues.md)