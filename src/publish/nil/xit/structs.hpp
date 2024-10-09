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

        operator Core&() const // NOLINT
        {
            return *ptr;
        }
    };

    C create_core(nil::service::S service);

    void set_cache_directory(Core& core, const std::filesystem::path& tmp_path);
}
