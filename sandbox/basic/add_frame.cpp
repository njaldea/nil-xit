#include "add_frame.hpp"

#include <nil/xit/add_frame.hpp>
#include <nil/xit/tagged/add_signal.hpp>
#include <nil/xit/tagged/add_value.hpp>
#include <nil/xit/unique/add_signal.hpp>
#include <nil/xit/unique/add_value.hpp>
#include <nil/xit/unique/post.hpp>

#include <iostream>

nil::xit::unique::Value<std::string>& add_base(nil::xit::Core& core)
{
    auto& frame = add_unique_frame(core, "base", "gui/Base.svelte");
    auto& value = add_value(
        frame,
        "value_0_1",
        []() { return std::string("hello world"); },
        [](std::string_view new_value) { std::cout << "value changed: " << new_value << std::endl; }
    );
    add_signal(
        frame,
        "signal-1",
        [&value]()
        {
            std::cout << "signal-1 is notified, forcing value_0_1 value" << std::endl;
            post(value, "new stuff here");
        }
    );
    add_signal(
        frame,
        "signal-2",
        [](const JSON& j)
        {
            std::cout << "signal-2 is notified" << std::endl;
            std::cout << j.buffer << std::endl;
        }
    );
    add_signal(
        frame,
        "signal-3",
        [](bool j)
        {
            std::cout << "signal-3 is notified" << std::endl;
            std::cout << j << std::endl;
        }
    );

    return value;
}

void add_tagged(nil::xit::Core& core)
{
    auto& frame = add_tagged_frame(core, "tagged", "gui/Tagged.svelte");
    add_value(
        frame,
        "tagged_value",
        [](std::string_view tag) -> std::int64_t
        {
            std::cout << "tagged_value getter: " << tag << std::endl;
            return 100;
        },
        [](std::string_view tag, std::int64_t v)
        {
            std::cout << "tagged_value setter: " << tag << std::endl;
            std::cout << v << std::endl;
        }
    );
    add_signal(
        frame,
        "tagged_signal",
        [](std::string_view tag, std::string_view value)
        { std::cout << tag << ":" << value << std::endl; }
    );
}

void add_group(nil::xit::Core& core)
{
    add_unique_frame(core, "group", "gui/Group.svelte");
}

void add_json_editor(nil::xit::Core& core)
{
    auto& frame = add_unique_frame(core, "json_editor", "gui/JsonEditor.svelte");
    auto json = std::make_shared<JSON>();
    json->buffer = R"({ "hello": "hello this is buffer" })";
    add_value(
        frame,
        "json_value",
        [json]() { return *json; },
        [json](const JSON& v)
        {
            *json = v;
            std::cout << v.buffer << std::endl;
        }
    );
}

void add_demo(nil::xit::Core& core)
{
    add_unique_frame(core, "demo", "gui/Demo.svelte");
}
