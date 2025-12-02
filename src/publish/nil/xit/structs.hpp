#pragma once

#include <nil/service/structs.hpp>
#include <nil/xalt/transparent_stl.hpp>

#include <filesystem>

namespace nil::xit
{
    struct Core;
    std::unique_ptr<Core, void (*)(Core*)> make_core(nil::service::IService& service);
    Core* create_core(nil::service::IService& service);
    void destroy_core(Core*);

    // setup the server to handle file requests
    // currently supports the following file format:
    //  -  .html
    //  -  .js
    //  -  .png
    //  -  .svg
    void setup_server(service::IWebService& server, std::vector<std::filesystem::path> asset_paths);

    struct FileInfo
    {
        std::string group;
        std::filesystem::path path;
    };

    void set_cache_directory(Core& core, std::filesystem::path tmp_path);

    void set_groups(Core& core, nil::xalt::transparent_umap<std::filesystem::path> groups);

    const nil::xalt::transparent_umap<std::filesystem::path>& get_groups(const Core& core);
}
