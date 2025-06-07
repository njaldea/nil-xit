# How it works

The C++ API requires a network service. This is provided by `nil/service` library. The GUI is not embedded in the binary; instead, it is served via an external interface.

A JavaScript library (nil-/xit) is provided to implement the UI using Svelte.
The svelte files are rebundled in the browser every time there is a change in any of its dependencies.
Because the Svelte files are separate from the C++ binary, changes to the UI do not require rebuilding the application.

## Example

Frame is used in the example below. For more information, see its [documentation](./02-Frames.md).

C++ Code

```cpp
#include <nil/xit.hpp>

#include <nil/service/ws/server/create.hpp>

#include <iostream>

int main()
{
    auto server = nil::service::ws::server::create({
        .host = "127.0.0.1",
        .port = 1101
    });
    auto core = nil::xit::make_core(server);

    set_groups(
        core,
        {
            {"base", "aliased path"},
            ...
        }
    );

    auto& frame = add_unique_frame(
        core,
        "frame-id",
        {
            .group="base",
            .path="/absolute/path/to/your/file.svelte"
        }
    );

    add_value(
        frame,
        "str_value",
        []() -> std::string
        {
            return "initial value";
        },
        [](std::string_view v)
        {
            std::cout << "value changed: " << v << std::endl;
        }
    );

    start(server);
    return 0;
}
```

Svelte Code

```svelte
<script>
    import { xit } from "@nil-/xit";

    const { values } = xit();

    const str_values = values.string("str_value", "world");
</script>

<input bind:value={$str_values} />
```

