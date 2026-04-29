# nil-xit

Python binding for **nil-xit** – a C++ bridge that connects backends to frontend UIs with a tiny, explicit protocol.

Define frames, values, and signals in C++; interact with them from Python. The client can be web, desktop, or any runtime that speaks the protocol.

## Features

- **Frames** – Group values and signals displayed by a UI component
- **Two flavors** – Unique (one instance) and tagged (keyed by a tag)
- **Values** – Bidirectional data binding with buffer types (bytes)
- **Signals** – Events from UI to backend with optional byte payloads

## Installation

```bash
pip install nil-xit
```

## Quick Start

```python
import nil_xit
import nil_service

# Create HTTP server on 127.0.0.1:1101
http = nil_service.create_http_server("127.0.0.1", 1101, 100 * 1024 * 1024)

# Setup server with asset paths
nil_xit.setup_server(http, ["assets"])

# Create websocket event service
ws = http.use_ws("/ws")

# Create core from web service and websocket event
core = nil_xit.create_core(http, ws)

# Add a frame
frame = core.add_unique_frame("index", "$base/gui/Demo.svelte")

# Add a value with getter and setter (buffer type)
def encode_value() -> bytes:
    return b"hello world"

def decode_value(data: bytes) -> None:
    print(f"value changed: {data.decode('utf-8')}")

value = frame.add_value("my_value", encode_value, decode_value)

# Add a signal handler
def on_button_click() -> None:
    print("Button clicked!")
    value.post(b"updated value")

frame.add_signal("click", on_button_click)

# Main loop
while True:
    http.poll()
```

For a complete example, see [sandbox/main.py](../../sandbox/main.py).

## Supported Types

The Python binding supports **buffer types** only:

- `bytes` – Binary data passed as `buffer_type<T>` in C++
  - Values use getter/setter functions that work with bytes
  - Signals pass optional byte payloads

## Architecture

The Python binding wraps the C API (`libnil-xit-c-api.so`) which provides access to:

- **unique frames** – Single-instance UI components
- **tagged frames** – Multi-instance UI components (keyed by tag)
- **values** – Bidirectional data binding with buffer types
- **signals** – Events from UI to backend with optional byte payloads

## Documentation

For detailed API documentation and more examples, visit:
- [C++ Documentation](https://github.com/njaldea/nil-xit)
- [C API Guide](https://github.com/njaldea/nil-xit/blob/master/docs/c-api.md)

## License

CC BY-NC-ND 4.0

## Support

For issues, questions, or contributions, visit the [GitHub repository](https://github.com/njaldea/nil-xit).
