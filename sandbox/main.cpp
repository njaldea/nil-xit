#include <nil/service/ws/server/create.hpp>

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
    auto& frame = add_frame(
        core,
        "base",
        std::filesystem::path(__FILE__).parent_path() / "gui/Base.svelte"
    );
    auto& binding = bind(
        frame,
        "binding_0_1",
        "hello world",
        [](std::string_view value) { std::cout << "value changed: " << value << std::endl; }
    );
    listen(
        frame,
        "listener-1",
        [&binding]()
        {
            std::cout << "listener-1 is notified, forcing binding_0_1 value" << std::endl;
            post(binding, "new stuff here");
        }
    );
    listen(
        frame,
        "listener-2",
        [](const JSON& j)
        {
            std::cout << "listener-2 is notified" << std::endl;
            std::cout << j.buffer << std::endl;
        }
    );
    listen(
        frame,
        "listener-3",
        [](bool j)
        {
            std::cout << "listener-3 is notified" << std::endl;
            std::cout << j << std::endl;
        }
    );

    return binding;
}

int main()
{
    std::thread th;
    auto server
        = nil::service::ws::server::create({.port = 1101, .buffer = 1024ul * 1024ul * 100ul});
    // https://xit-ui.vercel.app/view/{server or ip:port}/{frame id}
    on_ready(
        server,
        [](const auto& id)
        {
            std::cout << "ws ready: " << id.text << '\n';                                 //
            std::cout << " ui is at     : \n";                                            //
            std::cout << " -  https://xit-ui.vercel.app/view/localhost:1101/group\n";     //
            std::cout << " -  https://xit-ui.vercel.app/view/localhost:1101/base\n";      //
            std::cout << " -  https://xit-ui.vercel.app/view/localhost:1101/json_editor"; //
            std::cout << std::endl;
        }
    );

    auto core = nil::xit::create_core(server);

    {
        auto& frame = add_tagged_frame(
            core,
            "tagged",
            std::filesystem::path(__FILE__).parent_path() / "gui/Tagged.svelte"
        );
        nil::xit::bind(
            frame,
            "tagged_bind",
            std::function<std::int64_t(std::string_view)>( //
                [](std::string_view) { return 100; }
            ),
            std::function<void(std::string_view, std::int64_t)>( //
                [](std::string_view, std::int64_t v) { std::cout << v << std::endl; }
            )
        );
    }
    {
        add_frame(
            core,
            "group",
            std::filesystem::path(__FILE__).parent_path() / "gui/GroupUp.svelte"
        );
    }
    {
        auto& frame = add_frame(
            core,
            "json_editor", // frame id
            std::filesystem::path(__FILE__).parent_path() / "gui/JsonEditor.svelte"
        );
        bind(frame, "json_binding", JSON{.buffer = R"({ "hello": "hello this is buffer" })"});
    }
    {
        auto& str_bind = add_base(core);
        th = std::thread(
            [&]()
            {
                std::string line;
                std::cout << "input here: ";
                while (std::getline(std::cin, line))
                {
                    post(str_bind, line);
                    std::cout << "input here: ";
                }
            }
        );
    }

    start(server);
    th.join();
    return 0;
}
