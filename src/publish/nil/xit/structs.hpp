#pragma once

#include <nil/service/structs.hpp>

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

    C make_core(nil::service::S service);
    C make_core(nil::service::HTTPService& service);
    Core* create_core(nil::service::S service);
    Core* create_core(nil::service::HTTPService& service);
    void delete_core(Core*);

    struct HTTPServerOptions
    {
        std::filesystem::path source_path;
        std::uint16_t port = 0;
        std::uint64_t buffer_size = 4ul * 1024ul;
    };

    nil::service::H make_server(const HTTPServerOptions& options);

    void set_relative_directory(Core& core, const std::filesystem::path& directory);
    void set_cache_directory(Core& core, const std::filesystem::path& tmp_path);
}
