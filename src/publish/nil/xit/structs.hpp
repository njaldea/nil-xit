#pragma once

#include <nil/service/structs.hpp>
#include <nil/xalt/transparent_stl.hpp>

#include <filesystem>
#include <memory>

namespace nil::xit
{
    struct Core;

    struct C
    {
        std::unique_ptr<Core, void (*)(Core*)> ptr;
        operator Core&() const; // NOLINT(hicpp-explicit-conversions)
    };

    C make_core(nil::service::P service);
    Core* create_core(nil::service::P service);
    void delete_core(Core*);

    struct HTTPServerOptions
    {
        std::filesystem::path source_path;
        std::string host;
        std::uint16_t port = 0;
        std::uint64_t buffer_size = 4ul * 1024ul;
    };

    // setup the server to handle file requests
    // currently supports the following file format:
    //  -  .html
    //  -  .js
    //  -  .png
    //  -  .svg
    void setup_server(service::WebService& server, std::vector<std::filesystem::path> asset_paths);

    struct FileInfo
    {
        std::string group;
        std::filesystem::path path;
    };

    void set_ui_directories(
        Core& core,
        nil::xalt::transparent_umap<std::filesystem::path> ui_directories
    );

    void set_cache_directory(Core& core, std::filesystem::path tmp_path);
}
