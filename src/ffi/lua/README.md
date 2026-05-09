# nil-xit

LuaJIT binding for **nil-xit** - a C++ bridge that connects backends to frontend UIs with a tiny, explicit protocol.

Define frames, values, and signals in C++; interact with them from Lua. The client can be web, desktop, or any runtime that speaks the protocol.

## Features

- **Frames** - Group values and signals displayed by a UI component
- **Two flavors** - Unique (one instance) and tagged (keyed by a tag)
- **Values** - Bidirectional data binding with buffer types (bytes)
- **Signals** - Events from UI to backend with byte payloads (binary strings)
- **Options** - Key/value preprocessing hints for the frontend

## Quick Start

```lua
local nil_service = require("nil_service")
local nil_xit = require("nil_xit")

-- Create HTTP server on 127.0.0.1:1101
local http = nil_service.create_http_server("127.0.0.1", 1101, 100 * 1024 * 1024)

-- Setup server with asset paths
nil_xit.setup_server(http, { "assets" })

-- Create websocket event service
local ws = http:use_ws("/ws")

-- Create core from web service and websocket event
local core = nil_xit.create_core(http, ws)

-- Add a frame
local frame = core:add_unique_frame("index", { group = "base", path = "gui/Demo.svelte" })
frame:add_option("index_mode", "demo")

-- Add a value with getter and setter (buffer type)
local value = frame:add_value("my_value", {
    encode = function()
        return "hello world"
    end,
    decode = function(data)
        print("value changed: " .. tostring(data))
    end,
})

-- Add a signal handler (payload is a binary-safe string)
frame:add_signal("click", function(payload)
    print("Button clicked! payload size: " .. #payload)
    value:post("updated value")
end)

-- Add preprocessing options for the frontend
frame:add_option("theme", "dark")

-- Main loop
while true do
    http:poll()
end
```

For a complete example, see [sandbox/main.lua](../../sandbox/main.lua).

## Supported Types

The Lua binding supports **buffer types** only:

- `string` - Binary data passed as `buffer_type<T>` in C++
    - Values use `encode`/`decode` functions that return/receive strings
    - Signals pass byte payloads as strings (extra args can be ignored)

## Frames and FileInfo

Frame creation accepts an optional table with `group` and `path` fields that define
asset resolution for the frontend.

```lua
local frame = core:add_unique_frame("index", { group = "base", path = "gui/Demo.svelte" })
```

## Architecture

The Lua binding wraps the C API (`libnil-xit-c-api.so`) which provides access to:

- **unique frames** - Single-instance UI components
- **tagged frames** - Multi-instance UI components (keyed by tag)
- **values** - Bidirectional data binding with buffer types
- **signals** - Events from UI to backend with byte payloads (binary strings)

## Documentation

For detailed API documentation and more examples, visit:
- [C++ Documentation](https://github.com/njaldea/nil-xit)
- [C API Guide](https://github.com/njaldea/nil-xit/blob/master/docs/c-api.md)

## License

CC BY-NC-ND 4.0

## Support

For issues, questions, or contributions, visit the [GitHub repository](https://github.com/njaldea/nil-xit).
