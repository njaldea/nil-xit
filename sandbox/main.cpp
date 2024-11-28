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
    const auto source_path = std::filesystem::path(__FILE__).parent_path();

    auto http_server = nil::xit::make_server({
        .source_path = source_path / "node_modules/@nil-/xit",
        .host = "127.0.0.1",
        .port = 1101,
        .buffer_size = 1024ul * 1024ul * 100ul //
    });
    auto core = nil::xit::make_core(http_server);
    set_relative_directory(core, source_path);

    const auto tmp_dir = std::filesystem::temp_directory_path() / "sandbox";
    std::filesystem::remove_all(tmp_dir);
    set_cache_directory(core, tmp_dir);

    ::add_tagged(core);
    ::add_group(core);
    ::add_json_editor(core);
    ::add_demo(core);
    auto& str_value = ::add_base(core);

    std::thread th;
    on_ready(http_server, [&]() { th = run_input_loop(str_value); });

    start(http_server);
    th.join();

    return 0;
}
