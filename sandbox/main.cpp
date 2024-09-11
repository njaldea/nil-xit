#include <nil/service/ws/Server.hpp>

#include <nil/xit.hpp>

#include <iostream>
#include <thread>

int main()
{
    nil::service::ws::Server server({.port = 1101});
    // https://xit-ui.vercel.app/{server or ip:port}/{frame id}
    server.on_ready(                                                                      //
        [](const auto& id)                                                                //
        {                                                                                 //
            std::cout << "ready ws      : " << id.text << std::endl;                      //
            std::cout << " ui is at     : https://xit-ui.vercel.app/localhost:1101/id-1"; //
            std::cout << std::endl;                                                       //
        }
    );

    auto core = nil::xit::make_core(server);
    auto& frame = add_frame(
        core,
        "id-1", // frame id
        std::filesystem::path(__FILE__).parent_path() / "gui/Markup.svelte"
    );

    auto& str_bind = bind(
        frame,
        "binding_0_1",
        "hello world",
        // this is to test gui -> cpp data flow
        [](const std::string& value) { std::cout << "value changed: " << value << std::endl; }
    );

    // this is to test cpp -> gui data flow
    const std::thread th(
        [&]()
        {
            std::string line;
            std::cout << "input here: ";
            while (std::getline(std::cin, line))
            {
                // will not update the local data inside str_bind
                post(str_bind, line);

                // if local data needs to be overwritten, use `set` instead
                // this will update the local data without waiting for an update
                // from the UI.

                // set(str_bind, line);
                std::cout << "input here: ";
            }
        }
    );

    server.run();
    return 0;
}
