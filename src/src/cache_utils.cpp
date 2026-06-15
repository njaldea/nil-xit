#include "cache_utils.hpp"

#include "codec.hpp"

#include <nil/service/codec.hpp>
#include <nil/service/concat.hpp>

#include <flatbuffers/flatbuffer_builder.h>
#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/verifier.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iterator>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace nil::xit::fbs
{
    namespace
    {
        const nil::xit::FileInfo* resolve_frame_file_info(
            const Core& core,
            std::string_view frame_id,
            CacheType type
        )
        {
            switch (type)
            {
                case CacheType::unique:
                {
                    const auto frame_it = core.unique_frames.find(frame_id);
                    if (frame_it != core.unique_frames.end()
                        && frame_it->second.file_info.has_value())
                    {
                        return &*frame_it->second.file_info;
                    }
                    return nullptr;
                }
                case CacheType::tagged:
                {
                    const auto frame_it = core.tagged_frames.find(frame_id);
                    if (frame_it != core.tagged_frames.end()
                        && frame_it->second.file_info.has_value())
                    {
                        return &*frame_it->second.file_info;
                    }
                    return nullptr;
                }
            }

            return nullptr;
        }

        std::vector<flatbuffers::Offset<Alias>> build_alias_offsets(
            flatbuffers::FlatBufferBuilder& builder,
            const Core& core,
            const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>* groups,
            bool& valid
        )
        {
            std::vector<flatbuffers::Offset<Alias>> alias_offsets;
            if (groups == nullptr)
            {
                return alias_offsets;
            }

            alias_offsets.reserve(groups->size());
            for (const auto* group_name : *groups)
            {
                const auto group_sv = group_name->string_view();
                const auto it = core.groups.find(group_sv);
                if (it == core.groups.end())
                {
                    valid = false;
                    return {};
                }
                alias_offsets.emplace_back(CreateAlias(
                    builder,
                    builder.CreateString(group_sv),
                    builder.CreateString(it->second.string())
                ));
            }
            return alias_offsets;
        }

        std::vector<flatbuffers::Offset<flatbuffers::String>> build_group_offsets(
            flatbuffers::FlatBufferBuilder& builder,
            const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>>* groups
        )
        {
            std::vector<flatbuffers::Offset<flatbuffers::String>> group_offsets;
            if (groups == nullptr)
            {
                return group_offsets;
            }

            group_offsets.reserve(groups->size());
            for (const auto* group_name : *groups)
            {
                group_offsets.emplace_back(builder.CreateString(group_name->string_view()));
            }
            return group_offsets;
        }

        std::vector<flatbuffers::Offset<FileInfo>> build_file_offsets(
            flatbuffers::FlatBufferBuilder& builder,
            const flatbuffers::Vector<flatbuffers::Offset<FileInfo>>* files
        )
        {
            std::vector<flatbuffers::Offset<FileInfo>> file_offsets;
            if (files == nullptr)
            {
                return file_offsets;
            }

            file_offsets.reserve(files->size());
            for (const auto* info : *files)
            {
                const auto* metadata = info->metadata();
                const auto metadata_offset
                    = builder.CreateVector(metadata->data(), metadata->size());
                file_offsets.emplace_back(CreateFileInfo(
                    builder,
                    builder.CreateString(info->group()->string_view()),
                    builder.CreateString(info->path()->string_view()),
                    metadata_offset
                ));
            }
            return file_offsets;
        }

        bool build_alias_paths(
            const Core& core,
            const flatbuffers::Vector<flatbuffers::Offset<Alias>>* aliases,
            std::unordered_map<std::string_view, std::filesystem::path>& alias_paths
        )
        {
            if (aliases == nullptr)
            {
                return false;
            }

            alias_paths.clear();
            alias_paths.reserve(aliases->size());
            for (const auto* alias : *aliases)
            {
                const auto group_sv = alias->group()->string_view();
                const auto it = core.groups.find(group_sv);
                if (it == core.groups.end())
                {
                    return false;
                }

                if (it->second != std::filesystem::path(alias->path()->string_view()))
                {
                    return false;
                }

                alias_paths.emplace(group_sv, it->second);
            }

            return true;
        }

        bool cache_identity_matches(const FrameCache* cache, const nil::xit::FileInfo& file_info)
        {
            if (cache->group()->string_view() != file_info.group)
            {
                return false;
            }

            if (cache->path()->string_view() != file_info.path.c_str())
            {
                return false;
            }

            return true;
        }

        bool cache_options_match(
            const flatbuffers::Vector<flatbuffers::Offset<Option>>* cache_options,
            const nil::xalt::transparent_umap<std::string>& options
        )
        {
            if (cache_options == nullptr)
            {
                return false;
            }

            if (cache_options->size() != options.size())
            {
                return false;
            }

            return std::ranges::all_of(
                *cache_options,
                [&options](const auto* option)
                {
                    const auto key = option->key()->string_view();
                    const auto it = options.find(key);

                    if (it == options.end())
                    {
                        return false;
                    }

                    return it->second == option->value()->string_view();
                }
            );
        }

        bool cache_files_are_valid(
            const flatbuffers::Vector<flatbuffers::Offset<FileInfo>>* files,
            const std::unordered_map<std::string_view, std::filesystem::path>& alias_paths
        )
        {
            if (files == nullptr)
            {
                return false;
            }

            return std::ranges::all_of(
                *files,
                [&alias_paths](const auto& ff)
                {
                    const auto file_group = ff->group()->string_view();
                    const auto file_path = ff->path()->string_view();
                    const auto alias_it = alias_paths.find(file_group);
                    if (alias_it == alias_paths.end())
                    {
                        return false;
                    }

                    const auto target = alias_it->second / file_path;
                    if (!std::filesystem::exists(target))
                    {
                        return false;
                    }

                    const auto file_time
                        = std::filesystem::last_write_time(target).time_since_epoch().count();
                    using target_time_t = std::decay_t<decltype(file_time)>;
                    const auto* const metadata = ff->metadata();
                    const std::uint64_t size = metadata->size();
                    if (sizeof(target_time_t) != size)
                    {
                        return false;
                    }

                    const auto cached_target_time
                        = nil::service::codec<target_time_t>::deserialize(metadata->data(), size);
                    return cached_target_time == file_time;
                }
            );
        }

        bool build_cache_save_buffer(
            flatbuffers::FlatBufferBuilder& builder,
            const Core& core,
            const FrameCache* message,
            const nil::xit::FileInfo& frame_file_info
        )
        {
            bool groups_are_valid = true;
            auto alias_offsets
                = build_alias_offsets(builder, core, message->groups(), groups_are_valid);
            if (!groups_are_valid)
            {
                return false;
            }

            auto file_offsets = build_file_offsets(builder, message->files());
            auto group_offsets = build_group_offsets(builder, message->groups());
            auto option_offsets = build_option_offsets(builder, message->options());

            const auto cache_offset = CreateFrameCache(
                builder,
                builder.CreateString(message->id()->string_view()),
                builder.CreateString(frame_file_info.group.c_str()),
                builder.CreateString(frame_file_info.path.c_str()),
                builder.CreateVector(file_offsets),
                builder.CreateVector(group_offsets),
                builder.CreateVector(option_offsets),
                builder.CreateString(message->content()->string_view())
            );

            const auto groups_offset = builder.CreateVector(alias_offsets);
            const auto save_offset = CreateFrameCacheSave(builder, cache_offset, groups_offset);
            builder.Finish(save_offset);

            return true;
        }

        bool is_valid_cache_save(
            const Core& core,
            const FrameCacheSave* save,
            const nil::xalt::transparent_umap<std::string>& options,
            const nil::xit::FileInfo& file_info
        )
        {
            if (save == nullptr || save->cache() == nullptr)
            {
                return false;
            }

            const auto* cache = save->cache();
            std::unordered_map<std::string_view, std::filesystem::path> alias_paths;
            return build_alias_paths(core, save->groups(), alias_paths)
                && cache_identity_matches(cache, file_info)
                && cache_options_match(cache->options(), options)
                && cache_files_are_valid(cache->files(), alias_paths);
        }
    }

    std::string_view cache_dir(CacheType type)
    {
        switch (type)
        {
            case CacheType::unique:
                return "unique";
            case CacheType::tagged:
                return "tagged";
        }

        return "";
    }

    std::vector<flatbuffers::Offset<Option>> build_option_offsets(
        flatbuffers::FlatBufferBuilder& builder,
        const flatbuffers::Vector<flatbuffers::Offset<Option>>* options
    )
    {
        std::vector<flatbuffers::Offset<Option>> option_offsets;
        if (options == nullptr)
        {
            return option_offsets;
        }

        option_offsets.reserve(options->size());
        for (const auto* option : *options)
        {
            option_offsets.emplace_back(CreateOption(
                builder,
                builder.CreateString(option->key()->string_view()),
                builder.CreateString(option->value()->string_view())
            ));
        }
        return option_offsets;
    }

    std::vector<flatbuffers::Offset<Option>> build_option_offsets(
        flatbuffers::FlatBufferBuilder& builder,
        const nil::xalt::transparent_umap<std::string>& options
    )
    {
        std::vector<flatbuffers::Offset<Option>> option_offsets;
        option_offsets.reserve(options.size());
        for (const auto& [key, value] : options)
        {
            option_offsets.emplace_back(
                CreateOption(builder, builder.CreateString(key), builder.CreateString(value))
            );
        }
        return option_offsets;
    }

    std::string load_file(const std::filesystem::path& path)
    {
        std::ifstream f(path, std::ios::binary | std::ios::in);
        return {std::istream_iterator<char>{f >> std::noskipws}, std::istream_iterator<char>{}};
    }

    std::optional<std::string> load_valid_cache_content(
        const Core& core,
        CacheType type,
        std::string_view frame_id,
        const nil::xalt::transparent_umap<std::string>& options,
        const nil::xit::FileInfo& file_info
    )
    {
        if (!core.cache_location.has_value())
        {
            return std::nullopt;
        }

        const auto path = *core.cache_location / cache_dir(type) / frame_id;
        auto content = load_file(path);
        if (content.empty())
        {
            return std::nullopt;
        }

        flatbuffers::Verifier verifier(
            reinterpret_cast<const std::uint8_t*>(content.data()), // NOLINT
            content.size()
        );
        if (!verifier.VerifyBuffer<FrameCacheSave>())
        {
            return std::nullopt;
        }

        const auto* save = flatbuffers::GetRoot<FrameCacheSave>(content.data());
        if (!is_valid_cache_save(core, save, options, file_info))
        {
            return std::nullopt;
        }

        return content;
    }

    bool handle_frame_cache_message(
        Core& core,
        const void* data,
        std::uint64_t size,
        CacheType type
    )
    {
        if (!core.cache_location.has_value())
        {
            return false;
        }

        flatbuffers::Verifier verifier(static_cast<const std::uint8_t*>(data), size);
        if (!verifier.VerifyBuffer<FrameCache>())
        {
            return false;
        }

        const auto* message = flatbuffers::GetRoot<FrameCache>(data);
        if (message == nullptr)
        {
            return false;
        }

        const auto frame_id = message->id()->string_view();
        const auto* frame_file_info = resolve_frame_file_info(core, frame_id, type);
        if (frame_file_info == nullptr)
        {
            return false;
        }

        flatbuffers::FlatBufferBuilder builder;
        if (!build_cache_save_buffer(builder, core, message, *frame_file_info))
        {
            return false;
        }

        std::ofstream f(
            *core.cache_location / cache_dir(type) / frame_id,
            std::ios::binary | std::ios::out
        );
        f.write(
            reinterpret_cast<const char*>(builder.GetBufferPointer()), // NOLINT
            std::int64_t(builder.GetSize())
        );
        return true;
    }
}
