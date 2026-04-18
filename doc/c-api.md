# nil/xit C API Documentation

This document describes the C API for integrating with the nil/xit core and managing frames, values, and signals from C code.

---

## Overview

The C API provides opaque handles and functions for creating and managing the nil/xit core, unique/tagged frames, values, and signals. All handles are opaque structs and must be created/destroyed via the API. All API functions require valid, non-null handles unless otherwise documented.

---


## Core Management

- `nil_xit_core_create(nil_service_runnable run_service, nil_service_event event_service)`
  - Create a new core instance.
- `nil_xit_core_create_from_standalone(nil_service_standalone service)`
  - Create a core from a standalone service.
- `nil_xit_core_destroy(nil_xit_core core)`
  - Destroy a core instance and release resources.
- `nil_xit_set_cache_directory(nil_xit_core core, const char* tmp_path)`
  - Set the cache directory for the core.
- `nil_xit_set_groups(nil_xit_core core, const nil_xit_group_entry* groups, uint64_t size)`
  - Set the groups for the core.
- `nil_xit_setup_server(nil_service_web service, const char** asset_paths, size_t count)`
  - Set up a web server and register multiple asset paths (array of strings and count).

---

## Frame Management

- `nil_xit_core_add_unique_frame(nil_xit_core core, const char* id, const char* path)`
  - Add a unique frame to the core.
- `nil_xit_core_add_tagged_frame(nil_xit_core core, const char* id, const char* path)`
  - Add a tagged frame to the core.

---

## Value Management

- `nil_xit_unique_frame_add_value(nil_xit_unique_frame frame, const char* id, nil_xit_unique_value_accessor accessor)`
  - Add a value to a unique frame.
- `nil_xit_tagged_frame_add_value(nil_xit_tagged_frame frame, const char* id, nil_xit_tagged_value_accessor accessor)`
  - Add a value to a tagged frame.
- `nil_xit_unique_value_post(nil_xit_unique_frame_value value, const void* new_data, uint64_t new_data_size)`
  - Post a new value for a unique frame value.
- `nil_xit_tagged_value_post(nil_xit_tagged_frame_value value, const char* tag, const void* new_data, uint64_t new_data_size)`
  - Post a new value for a tagged frame value.

---


## Signal Management

- `nil_xit_unique_frame_add_signal(nil_xit_unique_frame frame, const char* id, nil_xit_unique_callback_info callback)`
  - Add a signal to a unique frame.
- `nil_xit_tagged_frame_add_signal(nil_xit_tagged_frame frame, const char* id, nil_xit_tagged_callback_info callback)`
  - Add a signal to a tagged frame.

---


## Lifecycle Hooks

- `nil_xit_unique_frame_on_load(nil_xit_unique_frame frame, nil_xit_unique_callback_info callback)`
- `nil_xit_tagged_frame_on_load(nil_xit_tagged_frame frame, nil_xit_tagged_callback_info callback)`
- `nil_xit_unique_frame_on_sub(nil_xit_unique_frame frame, nil_xit_unique_on_sub_info callback)`
- `nil_xit_tagged_frame_on_sub(nil_xit_tagged_frame frame, nil_xit_tagged_on_sub_info callback)`

---


## Structs

- `nil_xit_core`, `nil_xit_unique_frame`, `nil_xit_tagged_frame`, `nil_xit_unique_frame_value`, `nil_xit_tagged_frame_value` (opaque handles)
- `nil_xit_callback_info`, `nil_xit_unique_callback_info`, `nil_xit_tagged_callback_info` (callback registration)
- `nil_xit_unique_on_sub_info`, `nil_xit_tagged_on_sub_info` (subscription callbacks)
- `nil_xit_unique_value_accessor`, `nil_xit_tagged_value_accessor` (custom value encoding/decoding)
- `nil_xit_group_entry` (group configuration)

### Callback Structs

- `nil_xit_callback_info`: Generic callback info for events (legacy/compatibility).
- `nil_xit_unique_callback_info`: Callback info for unique frame events (use for unique frame signals and hooks).
- `nil_xit_tagged_callback_info`: Callback info for tagged frame events (use for tagged frame signals and hooks).

---

## Example

nil_xit_group_entry groups[] = { {"base", "/path/to/assets"} };
nil_xit_set_groups(core, groups, 1);
nil_xit_unique_frame uframe = nil_xit_core_add_unique_frame(core, "base", "/path/to/component.svelte");
nil_xit_core_destroy(core);
```c
// Example: Creating a core, setting up server, and adding a unique frame
nil_xit_core core = nil_xit_core_create(run_service, event_service);
const char* assets[] = { "/path/to/assets1", "/path/to/assets2" };
nil_xit_setup_server(web_service, assets, 2);
nil_xit_group_entry groups[] = { {"base", "/path/to/assets1"} };
nil_xit_set_groups(core, groups, 1);
nil_xit_unique_frame uframe = nil_xit_core_add_unique_frame(core, "base", "/path/to/component.svelte");
// ... add values, signals, etc ...
nil_xit_core_destroy(core);
```

---

For more details, see the header: `src/publish/nil/xit.h`
