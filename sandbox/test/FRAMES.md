## Frames

### There are 2 different types of Frames:
- input
- output

They are not interchangeable and must be used for their purpose

### There are 2 different types of input Frames:
- unique
    - maybe rename to common? or universal?
    - data it holds is the same between different tests
- tagged
    - data it holds is specialized for each test

### Registration of input Frames requires 3 things:
- frame id
- file name
- initializer

Frame initializer is a callable that returns the data that the frame will hold.
#### Currently, there are two callable creators:
- from_file
    - will read the file and will require the user to provide a predicate how to interpret the file
    - this is temporary. to be revamped later to not depend on full path.

```cpp
XIT_FRAME_TAGGED_INPUT(
    "input_frame",                                               // frame id
    "gui/InputFrame.svelte",                                     // ui file
    nil::xit::test::from_file(                                   //
        std::filesystem::path(__FILE__).parent_path() / "files", // file path
        "input_frame.json",                                      // file to load
        &as_json                                                 // how to interpret the file
    )                                                            //
);
```

- from_data
    - will just use the data provided to it

```cpp
XIT_FRAME_UNIQUE_INPUT(
    "slider_frame",                            // frame id
    "gui/Slider.svelte",                       // ui file
    nil::xit::test::from_data(Ranges{3, 2, 1}) // initializer
)
```

### Registration of output frames requries 3 things:
- frame id
- file name
- type

```cpp
XIT_FRAME_OUTPUT(
    "view_frame",           // frame id
    "gui/ViewFrame.svelte", // ui file
    nlohmann::json          // type of the output
);
```

### FRAME macros above provides an method to "bind" a value to the UI

- value with only value id
    - this will bind the whole data owned by the frame to the value id from the UI

```cpp
FRAME_MACRO(...)
    .value("value");
```

```html
<script>
    import { xit } from "@nil-/xit";
    const { values } = xit();
    const buf_value = values.json('value', {}, json_string);
</script>
```

- value with getter/setter or an accessor (struct with get/set)
    - this will bind portiong of the data owned by the frame to the value id from the UI

```cpp
FRAME_MACRO(...) // assuming this Frame owns a data of type "Type"
    .value(
        "value-1",
        Getter /* () => int64_t */,
        Setter /* (Type&, int64_t) => void */
    )
    .value("value-2", nil::xit::test::from_member(&Type::v2));
```

```html
<script>
    import { xit } from "@nil-/xit";
    const { values } = xit();
    const value_1 = values.number('value-1', 0);
    const value_2 = values.number('value-2', 1);
</script>
```

`nil::xit::test::from_member` is an accessor creator to simplify mapping for class members.

There is also `from_json_ptr` from the sandbox to demonstrate mapping for json_poitners

Note: Frame outputs only requires getter
