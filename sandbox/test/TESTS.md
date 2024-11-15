## Tests

### Registering tests requires 3 things
- Base Fixture inheriting from `nil::xit::test::Test<...>`
- Test Case name
- Directory location to load test tags

---

### Test id will follow this format

```cpp
XIT_TEST(Base, Case, "path/here"){}

// if path/here contains the following directories
// - a      -> Base.Case[a]
// - b      -> Base.Case[b]
```

---

### Base Fixture will dictate the frames it depend on

```cpp
using nil::xit::test::Test;
using nil::xit::test::InputFrames;
using nil::xit::test::OutputFrames;
using nil::xit::test::Frame;

using Sample = Test<
    InputFrames<
        Frame<nlohmann::json, "input_frame">,
        Frame<Ranges, "slider_frame">
    >,
    OutputFrames<
        Frame<nlohmann::json, "view_frame">
    >
>;
```

Where the Frame is the description of the Frame Type and the Frame ID
See Frame Registration [doc](./FRAMES.md) for more detail.

---

### `xit_inputs` and `xit_outputs`

These variables are only accessible inside the test body (not from the fixture)

```cpp
XIT_TEST(Sample, Demo, "files")
{
    const auto& [
        input_data, // nlohmann::json from "input_frame"
        ranges      // Ranges from "slider_frame"
    ] = xit_inputs;

    auto& [
        view        // nlohmann::json from "view_frame"
    ] = xit_outputs;
}