from __future__ import annotations

import select
import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
PY_FFI_DIR = REPO_ROOT / "src" / "ffi" / "python"

sys.path.insert(0, str(PY_FFI_DIR))

import nil_service
import nil_xit

SOURCE_DIR = str(Path(__file__).resolve().parent) + "/"

# Create HTTP server on 127.0.0.1:1101
http = nil_service.create_http_server("127.0.0.1", 1101, 100 * 1024 * 1024)

# Setup server with asset paths
nil_xit.setup_server(http, [
    "assets",
    "assets/xit/assets",
])

# Create websocket event service
ws = http.use_ws("/ws")

# Create core from web service and websocket event
core = nil_xit.create_core(http, ws)

# Set groups
core.set_groups({
    "base":       SOURCE_DIR,
    "components": SOURCE_DIR + "gui/components",
})

# Initialize frames
core.add_unique_frame("index", "$base/gui/Demo.svelte")
base_frame = core.add_unique_frame("base", "$base/gui/Base.svelte")
core.add_unique_frame("group", "$base/gui/Group.svelte")
core.add_unique_frame("json_editor", "$base/gui/JsonEditor.svelte")
core.add_tagged_frame("tagged", "$base/gui/Tagged.svelte")

# Mutable state for the string value
str_value_g = b"hello world"


def encode_value() -> bytes:
    return str_value_g


def decode_value(data: bytes) -> None:
    global str_value_g
    str_value_g = data
    sys.stdout.write("value changed: " + str_value_g.decode("utf-8", errors="replace") + "\n")
    sys.stdout.flush()


str_value = base_frame.add_value("value_0_1", encode_value, decode_value)


def on_signal_1() -> None:
    sys.stdout.write("signal-1 is notified, forcing value_0_1 value\n")
    sys.stdout.flush()
    str_value.post(b"new stuff here")


def on_signal_2() -> None:
    sys.stdout.write("signal-2 is notified\n")
    sys.stdout.flush()


def on_signal_3() -> None:
    sys.stdout.write("signal-3 is notified\n")
    sys.stdout.flush()


base_frame.add_signal("signal-1", on_signal_1)
base_frame.add_signal("signal-2", on_signal_2)
base_frame.add_signal("signal-3", on_signal_3)


def on_ready(id: nil_service.ID) -> None:
    print("http://" + id.to_string())


http.on_ready(on_ready)

# Main loop
while True:
    http.poll()

    readable, _, _ = select.select([sys.stdin], [], [], 0.001)
    if readable:
        line = sys.stdin.readline()
        if not line:
            break
        line = line.rstrip("\n")
        if line in ("quit", "exit"):
            break
        str_value.post(line.encode("utf-8"))
        sys.stdout.write("input here: ")
        sys.stdout.flush()
    else:
        time.sleep(0.001)