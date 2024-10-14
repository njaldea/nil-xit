#include <nil/xit.hpp>

#include <iostream>
#include <thread>

struct JSON
{
    std::string buffer;
};

namespace nil::xit
{
    template <>
    struct buffer_type<JSON>
    {
        static JSON deserialize(const void* data, std::uint64_t size)
        {
            return JSON{std::string(static_cast<const char*>(data), size)};
        }

        static std::vector<std::uint8_t> serialize(const JSON& value)
        {
            return {value.buffer.begin(), value.buffer.end()};
        }
    };
}

auto& add_base(nil::xit::Core& core)
{
    auto& frame = add_unique_frame(core, "base", "gui/Base.svelte");
    auto& value = add_value(
        frame,
        "value_0_1",
        "hello world",
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

int main()
{
    const auto source_path = std::filesystem::path(__FILE__).parent_path();

    auto http_server = nil::xit::make_server({
        .source_path = source_path,
        .port = 1101,
        .buffer_size = 1024ul * 1024ul * 100ul //
    });
    auto core = nil::xit::make_core(http_server);
    set_relative_directory(core, source_path);

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
    {
        add_unique_frame(core, "group", "gui/GroupUp.svelte");
    }
    {
        auto& frame = add_unique_frame(core, "json_editor", "gui/JsonEditor.svelte");
        add_value(
            frame,
            "json_value",
            JSON{.buffer = R"({ "hello": "hello this is buffer" })"},
            [](const JSON& v) { std::cout << v.buffer << std::endl; }
        );
    }

    std::thread th;
    {
        auto& str_value = add_base(core);
        th = std::thread(
            [&]()
            {
                std::string line;
                std::cout << "input here: ";
                while (std::getline(std::cin, line))
                {
                    post(str_value, line);
                    std::cout << "input here: ";
                }
            }
        );
    }

    {
        add_unique_frame(core, "demo", "gui/Demo.svelte");
    }

    std::filesystem::remove_all(std::filesystem::temp_directory_path() / "sandbox");
    set_cache_directory(core, std::filesystem::temp_directory_path() / "sandbox");

    start(http_server);
    th.join();
    return 0;
}
