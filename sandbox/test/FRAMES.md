## Frames

### There are 2 different types of Frames:
- input
- output

They are not interchangeable and must be used for their purpose

---

### There are 2 different types of input Frames:
- unique
    - maybe rename to common? or universal?
    - data it holds is the same between different tests
- tagged
    - data it holds is specialized for each test

---

### Registration of input Frames requires 3 things:
- frame id
- file name
- initializer
    - either the value to be owned by the frame or,
    - a callable that receives tag and returns the value to be owned by the frame

```cpp
XIT_FRAME_UNIQUE_INPUT(frame_id, "ui/file.svelte", Ranges(3, 2, 1));
XIT_FRAME_UNIQUE_INPUT(frame_id, "ui/file.svelte", [](std::string_view tag){ return Ranges(3, 2, 1); });
```

---

### Registration of output frames requries 3 things:
- frame id
- file name
- type

```cpp
XIT_FRAME_OUTPUT(frame_id, "ui/file.svelte", nlohmann::json);
```

---

### FRAME macros above provides an method to "bind" a value to the UI

- value with only value id
    - this will bind the whole data owned by the frame to the value id from the UI

```cpp
FRAME_MACRO(...) // assuming this Frame owns a data of type "std::string"
    .value("value-id");
```

```html
<script>
    import { xit } from "@nil-/xit";
    const { values } = xit();
    const buf_value = values.string('value-id', "default_value");
</script>
```

- value with getter/setter
- value with pointer to member
- value with an accessor type (struct with get/set)
    - these will bind portion of the data owned by the frame to the value id from the UI

```cpp
FRAME_MACRO(...) // assuming this Frame owns a data of type "Type"
    .value("value-1", Getter, Setter)
    .value("value-2", &Type::v2);
```

```html
<script>
    import { xit } from "@nil-/xit";
    const { values } = xit();
    const value_1 = values.number('value-1', 0);
    const value_2 = values.number('value-2', 1);
</script>
```

There is also `from_json_ptr` from the sandbox to demonstrate mapping for json_poitners

Note: Frame outputs only requires getter
