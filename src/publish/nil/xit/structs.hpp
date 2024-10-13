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

    inline C make_core(nil::service::S service)
    {
        return {std::unique_ptr<Core, void (*)(Core*)>(create_core(service), &delete_core)};
    }

    void set_cache_directory(Core& core, const std::filesystem::path& tmp_path);
}
