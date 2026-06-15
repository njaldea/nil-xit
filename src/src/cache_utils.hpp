#pragma once

#include "messages/message.fbs.h"
#include "structs.hpp"

#include <nil/xalt/transparent_stl.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace nil::xit
{
    struct FileInfo;
}

namespace nil::service
{
    class ID;
}

namespace nil::xit::fbs
{
    enum class CacheType
    {
        unique,
        tagged,
    };

    std::string_view cache_dir(CacheType type);

    std::string load_file(const std::filesystem::path& path);

    std::optional<std::string> load_valid_cache_content(
        const Core& core,
        CacheType type,
        std::string_view frame_id,
        const nil::xalt::transparent_umap<std::string>& options,
        const nil::xit::FileInfo& file_info
    );

    std::vector<flatbuffers::Offset<Option>> build_option_offsets(
        flatbuffers::FlatBufferBuilder& builder,
        const flatbuffers::Vector<flatbuffers::Offset<Option>>* options
    );

    std::vector<flatbuffers::Offset<Option>> build_option_offsets(
        flatbuffers::FlatBufferBuilder& builder,
        const nil::xalt::transparent_umap<std::string>& options
    );

    bool handle_frame_cache_message(
        Core& core,
        const void* data,
        std::uint64_t size,
        CacheType type
    );
}
