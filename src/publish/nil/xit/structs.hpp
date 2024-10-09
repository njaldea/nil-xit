#pragma once

#include <nil/service/structs.hpp>

#include <filesystem>
#include <memory>

namespace nil::xit
{
    template <typename T>
    struct Binding;
    template <typename T>
    struct TaggedBinding;

    struct Frame;
    struct TaggedFrame;
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

    template <typename T>
    struct buffer_type
    {
        static T deserialize(const void* data, std::uint64_t size) = delete;
        static std::vector<std::uint8_t> serialize(const T& value) = delete;
    };

    void set_cache_directory(Core& core, const std::filesystem::path& tmp_path);
}
