#include <filesystem>

#include "xit_gtest.hpp" // IWYU pragma: keep
#include "xit_test.hpp"

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

using nil::xit::gtest::from_file;

XIT_USE_DIRECTORY(std::filesystem::path(__FILE__).parent_path());

XIT_FRAME_TAGGED_INPUT(
    input_frame,
    "gui/InputFrame.svelte",
    from_file("files", "input_frame.json", &as_json)
)
    .value("value");

XIT_FRAME_UNIQUE_INPUT(slider_frame, "gui/Slider.svelte", Ranges(3, 2, 1))
    .value("value-1", &Ranges::v1)
    .value("value-2", &Ranges::v2)
    .value("value-3", &Ranges::v3);

XIT_FRAME_OUTPUT(view_frame, "gui/ViewFrame.svelte", nlohmann::json)
    .value("value-x", from_json_ptr("/x"))
    .value("value-y", from_json_ptr("/y"));

// TODO: will this be enough? this will require that frame registration is visible to the test.
// if there are multiple files, is inlining the registration enough? or are there other options?
// NOLINTNEXTLINE
#define FRAME(X) nil::xit::test::Frame<std::remove_cvref_t<decltype(xit_test_frame_##X)>::type, #X>

using InputFrame = FRAME(input_frame);
using SliderFrame = FRAME(slider_frame);
using ViewFrame = FRAME(view_frame);

using Sample = nil::xit::test::Test<
    nil::xit::test::InputFrames<InputFrame, SliderFrame>,
    nil::xit::test::OutputFrames<ViewFrame>>;

XIT_TEST(Sample, Demo, "files")
{
    const auto& [input_data, ranges] = xit_inputs;

    auto tag = std::string(input_data["x"][2]);
    std::cout << "run (test) " << tag << std::endl;

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

    // installation step. will not be visible to the end user.
    {
        auto& instance = nil::xit::gtest::get_instance();
        instance.frame_builder.install(app, instance.relative_path);
        instance.test_builder.install(app, instance.relative_path);
    }

    // TODO: install like others
    {
        using nil::xit::test::from_data;
        using j = nlohmann::json;
        auto& f = add_unique_frame(app.xit, "demo", source_path / "gui/Main.svelte");

        add_value(f, "tags", from_data(j(app.installed_tags())));
        add_value(f, "view", from_data(j::array({"view_frame"})));
        add_value(f, "pane", from_data(j::array({"slider_frame", "input_frame"})));
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
