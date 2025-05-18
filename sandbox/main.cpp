#include <nil/service.hpp>
#include <nil/xit.hpp>

#include "add_frame.hpp"

#include <iostream>
#include <thread>

std::thread run_input_loop(nil::xit::unique::Value<std::string>& str_value)
{
    return std::thread(
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

int main()
{
    const auto source_path = std::filesystem::path(std::filesystem::path(__FILE__).parent_path());

    auto server = nil::service::http::server::create({
        .host = "127.0.0.1",
        .port = 1101,
        .buffer = 1024ul * 1024ul * 100ul //
    });

    nil::xit::setup_server(server, {source_path / "node_modules/@nil-/xit/assets"});
    auto core = nil::xit::make_core(use_ws(server, "/ws"));
    set_ui_directories(
        core,
        {{"base", source_path}, {"components", source_path / "gui/components"}}
    );

    const auto tmp_dir = std::filesystem::temp_directory_path() / "sandbox";
    std::filesystem::remove_all(tmp_dir);
    set_cache_directory(core, tmp_dir);

    ::add_tagged(core);
    ::add_group(core);
    ::add_json_editor(core);
    ::add_demo(core);
    auto& str_value = ::add_base(core);

    std::thread th;
    on_ready(server, [&]() { th = run_input_loop(str_value); });

    start(server);
    th.join();

    return 0;
}
