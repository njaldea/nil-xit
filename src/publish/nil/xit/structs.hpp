#pragma once

#include <nil/service/structs.hpp>

#include <filesystem>
#include <memory>

namespace nil::xit
{
    struct Core;

    Core* create_core(nil::service::S service);
    void delete_core(Core*);

    struct C
    {
        std::unique_ptr<Core, void (*)(Core*)> ptr;

        operator Core&() const // NOLINT
        {
            return *ptr;
        }
    };

    void set_relative_directory(Core& core, const std::filesystem::path& directory);
    void set_cache_directory(Core& core, const std::filesystem::path& tmp_path);

    inline C make_core(nil::service::S service)
    {
        return {std::unique_ptr<Core, void (*)(Core*)>( //
            create_core(service),
            &delete_core
        )};
    }

    inline C make_core(nil::service::HTTPService& service)
    {
        auto c = C{std::unique_ptr<Core, void (*)(Core*)>( //
            create_core(use_ws(service, "/ws")),
            &delete_core
        )};
        return c;
    }

    struct HTTPServerOptions
    {
        std::filesystem::path source_path;
        std::uint16_t port = 0;
        std::uint64_t buffer_size = 4ul * 1024ul;
    };

    nil::service::H make_server(const HTTPServerOptions& options);
}
