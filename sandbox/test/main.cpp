#include <filesystem>

#include "xit_gtest.hpp" // IWYU pragma: keep

#include <nlohmann/json.hpp>

#include <iostream>
#include <string_view>
#include <utility>

namespace nil::xit
{
    // this is necessary when publishing a custom data through the network going to the UI
    template <>
    struct buffer_type<nlohmann::json>
    {
        static nlohmann::json deserialize(const void* data, std::uint64_t size)
        {
            return nlohmann::json::parse(std::string_view(static_cast<const char*>(data), size));
        }

        static std::vector<std::uint8_t> serialize(const nlohmann::json& value)
        {
            auto s = value.dump();
            return {s.begin(), s.end()};
        }
    };
}

struct Ranges
{
    std::int64_t v1;
    std::int64_t v2;
    std::int64_t v3;

    // necessary for nil::gate edge dirty mechanism.
    bool operator==(const Ranges& o) const
    {
        return v1 == o.v1 && v2 == o.v2 && v3 == o.v3;
    }
};

nlohmann::json as_json(std::istream& iss)
{
    return nlohmann::json::parse(iss);
}

Ranges as_range(std::istream& iss)
{
    auto r = Ranges{};
    auto c = char{};
    iss >> c;
    iss >> r.v1;
    iss >> c;
    iss >> r.v2;
    iss >> c;
    iss >> r.v3;
    iss >> c;
    return r;
}

template <typename T = nlohmann::json>
auto from_json_ptr(const std::string& json_ptr)
{
    struct Accessor
    {
        T get(const nlohmann::json& data) const
        {
            return data[json_ptr];
        }

        void set(nlohmann::json& data, T new_data) const
        {
            data[json_ptr] = std::move(new_data);
        }

        nlohmann::json::json_pointer json_ptr;
    };

    return Accessor{nlohmann::json::json_pointer(json_ptr)};
}

// temporary macro to set relative path
XIT_USE_DIRECTORY(std::filesystem::path(__FILE__).parent_path());

// This Frame Input is going to be independent for each test that requires it
XIT_FRAME_TAGGED_INPUT(
    "input_frame",                                               // frame id
    "gui/InputFrame.svelte",                                     // ui file
    nil::xit::test::from_file(                                   //
        std::filesystem::path(__FILE__).parent_path() / "files", // file path
        "input_frame.json",                                      // file to load
        &as_json                                                 // how to interpret the file
    )                                                            //
)                                                                //
    .value("value"); //  without additional information, the whole data owned by this frame is bound
                     //  to "value" of the UI

// This Frame Input is going to be common for all tests that requires it
XIT_FRAME_UNIQUE_INPUT(
    "slider_frame",                            // frame id
    "gui/Slider.svelte",                       // ui file
    nil::xit::test::from_data(Ranges{3, 2, 1}) // initializer
)
    // from_file(source_path / "files", "slider_frame.json", &as_range)
    // value-1 is bound to v1 property of Ranges object that is owned by this frame
    .value("value-1", nil::xit::test::from_member(&Ranges::v1))
    .value("value-2", nil::xit::test::from_member(&Ranges::v2))
    .value("value-3", nil::xit::test::from_member(&Ranges::v3));

// This Frame Output is going to be specific for each test.
XIT_FRAME_OUTPUT(
    "view_frame",           // frame id
    "gui/ViewFrame.svelte", // ui file
    nlohmann::json          // type of the output
)
    // value-x is bound to the data referred by the json pointer below
    .value("value-x", from_json_ptr("/x"))
    .value("value-y", from_json_ptr("/y"));

// These are frames available as registered above. There will be runtime check for type
// matching/compatibility.
// TODO: can this be done differently?
using InputFrame = nil::xit::test::Frame<nlohmann::json, "input_frame">;
using SliderFrame = nil::xit::test::Frame<Ranges, "slider_frame">;
using ViewFrame = nil::xit::test::Frame<nlohmann::json, "view_frame">;

// This would be the base class of the test and will be the test suite name
using Sample = nil::xit::test::Test<
    nil::xit::test::InputFrames<InputFrame, SliderFrame>,
    nil::xit::test::OutputFrames<ViewFrame>>;

XIT_TEST(Sample, Demo, "files")
{
    // destructure to get all of the inputs
    // order is described by "Sample" type
    const auto& [input_data, ranges] = xit_inputs;

    auto tag = std::string(input_data["x"][2]);
    std::cout << "run (test) " << tag << std::endl;

    // destructure to get all of the outputs and modify accordingly
    // these are defaultly initialized
    // exception handling is not yet emplaced so don't throw from test
    auto& [view] = xit_outputs;
    view = input_data; // copy the data and mutate as necessary
    view["y"][0] = input_data["y"][0].get<std::int64_t>() * ranges.v1;
    view["y"][1] = input_data["y"][1].get<std::int64_t>() * ranges.v2;
    view["y"][2] = input_data["y"][2].get<std::int64_t>() * ranges.v3;
}

// TODO: hide this main from user
int main()
{
    const auto source_path = std::filesystem::path(__FILE__).parent_path();
    const auto http_server = nil::xit::make_server({
        .source_path = source_path.parent_path() / "node_modules/@nil-/xit",
        .port = 1101,
        .buffer_size = 1024ul * 1024ul * 100ul //
    });
    nil::xit::test::App app(use_ws(http_server, "/ws"), "nil-xit-gtest");

    // TODO: make this dynamic so it can add tests on demand and modify the pane/view dependening on
    // the test that is loaded.
    //  This is going to be not visible to the end user
    {
        using j = nlohmann::json;
        auto& frame = add_unique_frame(app.xit, "demo", source_path / "gui/Main.svelte");
        add_value(
            frame,
            "tags",
            []() { return j::array({"", "Sample.Demo[a]", "Sample.Demo[b]"}); }
        );
        add_value(frame, "view", []() { return j::parse(R"(["view_frame"])"); });
        add_value(frame, "pane", []() { return j::parse(R"(["slider_frame", "input_frame"])"); });
    }

    // installation step. will not be visible to the end user.
    {
        auto& instance = nil::xit::gtest::get_instance();
        instance.frame_builder.install(app, instance.relative_path);
        instance.test_builder.install(app, instance.relative_path);
    }

    // TODO:
    //  - type erasure due to runtime storage
    //      - resolve in runtime if the frame is compatible to the test

    start(http_server);
    return 0;
}

// ISSUES:
//  -   loading an input is lazy but once loaded, any unique frame update will trigger rerun of
//  nodes those loaded frames
//  -   where should be the "demo" frame defined? main? static like normal frames?
//  -   do i need signals? for an editor, probably.
//  -   a lot of namespaces including the utility methods like (from_file, from_data)
