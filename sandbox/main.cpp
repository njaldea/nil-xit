#include <nil/service/http/server/create.hpp>

#include <nil/service/structs.hpp>
#include <nil/xit.hpp>

#include <fstream>
#include <iostream>
#include <thread>

const auto source_path = std::filesystem::path(__FILE__).parent_path();

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
    auto& frame = add_unique_frame(core, "base", source_path / "gui/Base.svelte");
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
    std::thread th;
    constexpr auto buffer_size = 1024ul * 1024ul * 100ul;
    // auto server = nil::service::ws::server::create({.port = 1101, .buffer = buffer_size});
    auto http_server = nil::service::http::server::create({.port = 1101, .buffer = buffer_size});

    use(http_server,
        "/",
        "text/html",
        [](std::ostream& os)
        {
            std::ifstream file(source_path / "index.html", std::ios::binary);
            os << file.rdbuf();
        });

    use(http_server,
        "/xit/index.js",
        "application/javascript",
        [](std::ostream& os)
        {
            std::ifstream file(source_path / "xit/index.js", std::ios::binary);
            os << file.rdbuf();
        });

    use(http_server,
        "/assets/bundler.js",
        "application/javascript",
        [](std::ostream& os)
        {
            std::ifstream file(source_path / "assets/bundler.js", std::ios::binary);
            os << file.rdbuf();
        });

    auto server = use_ws(http_server, "/ws/");
    auto core = nil::xit::make_core(server);

    {
        auto& frame = add_tagged_frame(core, "tagged", source_path / "gui/Tagged.svelte");
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
        add_unique_frame(core, "group", source_path / "gui/GroupUp.svelte");
    }
    {
        auto& frame = add_unique_frame(core, "json_editor", source_path / "gui/JsonEditor.svelte");
        add_value(
            frame,
            "json_value",
            JSON{.buffer = R"({ "hello": "hello this is buffer" })"},
            [](const JSON& v) { std::cout << v.buffer << std::endl; }
        );
    }
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
        add_unique_frame(core, "demo", source_path / "gui/Demo.svelte");
    }

    std::filesystem::remove_all(std::filesystem::temp_directory_path() / "sandbox");
    set_cache_directory(core, std::filesystem::temp_directory_path() / "sandbox");

    start(http_server);
    th.join();
    return 0;
}
