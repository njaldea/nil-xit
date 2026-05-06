#include "codec.hpp" // IWYU pragma: keep
#include "structs.hpp"
#include "tagged/utils.hpp" // IWYU pragma: keep
#include "unique/utils.hpp" // IWYU pragma: keep

#include <nil/xalt/literal.hpp>
#include <nil/xalt/transparent_stl.hpp>

#include <nil/service/codec.hpp>
#include <nil/service/concat.hpp>
#include <nil/service/consume.hpp>
#include <nil/service/map.hpp>
#include <nil/service/structs.hpp>

#include <flatbuffers/buffer.h>
#include <flatbuffers/flatbuffer_builder.h>
#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/verifier.h>

#include <filesystem>
#include <fstream>
#include <ios>
#include <optional>
#include <type_traits>

namespace nil::xit::fbs
{
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

    std::string load_file(const std::filesystem::path& path)
    {
        std::ifstream f(path, std::ios::binary | std::ios::in);
        return {std::istream_iterator<char>{f >> std::noskipws}, std::istream_iterator<char>{}};
    }

    bool validate_cache(
        Core& core,
        const nil::service::ID& id,
        const std::filesystem::path& cache_path,
        const nil::xalt::transparent_umap<std::string>& options,
        const nil::xit::FileInfo& file_info,
        std::optional<std::string_view> tag = {}
    )
    {
        if (!std::filesystem::exists(cache_path))
        {
            return false;
        }

        // Parse cached save payload.
        const auto content = load_file(cache_path);
        const auto* save = flatbuffers::GetRoot<FrameCacheSave>(content.data());
        if (save == nullptr || save->cache() == nullptr)
        {
            return false;
        }

        // Validate saved alias groups against current core groups.
        const auto* aliases = save->groups();
        if (aliases == nullptr)
        {
            return false;
        }

        auto alias_path_for = [aliases](std::string_view g) -> std::optional<std::filesystem::path>
        {
            for (const auto* alias : *aliases)
            {
                if (alias->group()->string_view() == g)
                {
                    return std::filesystem::path(alias->path()->string_view());
                }
            }
            return std::nullopt;
        };

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
        }

        const auto* cache = save->cache();
        const auto* cache_options = cache->options();
        if (cache_options == nullptr)
        {
            return false;
        }
        if (cache_options->size() != options.size())
        {
            return false;
        }
        for (const auto* option : *cache_options)
        {
            const auto key = option->key()->string_view();
            const auto it = options.find(key);
            if (it == options.end())
            {
                return false;
            }
            if (it->second != option->value()->string_view())
            {
                return false;
            }
        }

        // Validate file timestamps for cache contents.
        for (const auto& ff : *cache->files())
        {
            const auto file_group = ff->group()->string_view();
            const auto file_path = ff->path()->string_view();
            const auto alias_path = alias_path_for(file_group);
            if (!alias_path.has_value())
            {
                return false;
            }

            const auto target = *alias_path / file_path;
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
            if (cached_target_time != file_time)
            {
                return false;
            }
        }

        if (tag.has_value())
        {
            flatbuffers::FlatBufferBuilder builder;
            auto option_offsets = build_option_offsets(builder, cache_options);
            builder.Finish(CreateTaggedFrameInfoResponse(
                builder,
                builder.CreateString(cache->id()),
                builder.CreateString(tag.value()),
                builder.CreateString(file_info.group),
                builder.CreateString(file_info.path.c_str()),
                builder.CreateVector(option_offsets),
                builder.CreateString(cache->content())
            ));

            const auto header = MessageType_Server_Tagged_FrameInfo_Response;
            auto payload = nil::service::concat(header, builder);
            core.msg_service->send(id, std::move(payload));
        }
        else
        {
            flatbuffers::FlatBufferBuilder builder;
            auto option_offsets = build_option_offsets(builder, cache_options);
            builder.Finish(CreateUniqueFrameInfoResponse(
                builder,
                builder.CreateString(cache->id()),
                builder.CreateString(file_info.group),
                builder.CreateString(file_info.path.c_str()),
                builder.CreateVector(option_offsets),
                builder.CreateString(cache->content())
            ));

            const auto header = MessageType_Server_Unique_FrameInfo_Response;
            auto payload = nil::service::concat(header, builder);
            core.msg_service->send(id, std::move(payload));
        }
        return true;
    }

    void handle(Core& core, const nil::service::ID& id, const UniqueFrameInfoRequest& message)
    {
        const auto it = core.unique_frames.find(message.id()->string_view());
        if (it != core.unique_frames.end())
        {
            const auto& [frame_id, frame] = *it;

            if (!frame.file_info.has_value())
            {
                return;
            }

            if (core.cache_location.has_value())
            {
                const auto dir = *core.cache_location / "unique" / frame_id;
                if (validate_cache(core, id, dir.c_str(), frame.options, *frame.file_info))
                {
                    return;
                }
            }

            flatbuffers::FlatBufferBuilder builder;
            auto option_offsets = build_option_offsets(builder, frame.options);
            builder.Finish(CreateUniqueFrameInfoResponse(
                builder,
                builder.CreateString(frame_id),
                builder.CreateString(frame.file_info->group.c_str()),
                builder.CreateString(frame.file_info->path.c_str()),
                builder.CreateVector(option_offsets)
            ));

            const auto header = MessageType_Server_Unique_FrameInfo_Response;
            auto payload = nil::service::concat(header, builder);
            core.msg_service->send(id, std::move(payload));
        }
    }

    void handle(Core& core, const nil::service::ID& id, const TaggedFrameInfoRequest& message)
    {
        const auto it = core.tagged_frames.find(message.id()->string_view());
        if (it != core.tagged_frames.end())
        {
            const auto& [frame_id, frame] = *it;

            if (!frame.file_info.has_value())
            {
                return;
            }

            if (core.cache_location.has_value())
            {
                const auto dir = *core.cache_location / "tagged" / frame_id;
                const auto tag = message.tag()->string_view();
                if (validate_cache(core, id, dir.c_str(), frame.options, *frame.file_info, tag))
                {
                    return;
                }
            }

            flatbuffers::FlatBufferBuilder builder;
            auto option_offsets = build_option_offsets(builder, frame.options);
            builder.Finish(CreateTaggedFrameInfoResponse(
                builder,
                builder.CreateString(frame_id),
                builder.CreateString(message.tag()),
                builder.CreateString(frame.file_info->group.c_str()),
                builder.CreateString(frame.file_info->path.c_str()),
                builder.CreateVector(option_offsets)
            ));

            const auto header = MessageType_Server_Tagged_FrameInfo_Response;
            auto payload = nil::service::concat(header, builder);
            core.msg_service->send(id, std::move(payload));
        }
    }

    void handle(Core& core, const nil::service::ID& id, const FileRequest& request)
    {
        const auto group = request.group()->string_view();
        const auto path = request.path()->string_view();

        const auto it = core.groups.find(group);
        if (it == core.groups.end())
        {
            return;
        }

        const auto target = it->second / path;
        const auto content = load_file(target);

        const auto target_time
            = std::filesystem::last_write_time(target).time_since_epoch().count();
        const auto metadata = nil::service::concat(target_time);

        flatbuffers::FlatBufferBuilder builder;
        builder.Finish(CreateFileResponse(
            builder,
            builder.CreateString(group),
            builder.CreateString(path),
            builder.CreateString(content),
            builder.CreateVector(metadata)
        ));

        auto header = MessageType_Server_File_Response;
        auto payload = nil::service::concat(header, builder);
        core.msg_service->send(id, std::move(payload));
    }

    void handle(Core& core, const nil::service::ID& /* id */, const UniqueFrameLoaded& msg)
    {
        const auto it = core.unique_frames.find(msg.id()->string_view());
        if (it != core.unique_frames.end())
        {
            load(it->second);
        }
    }

    void handle(Core& core, const nil::service::ID& /* id */, const TaggedFrameLoaded& msg)
    {
        const auto it = core.tagged_frames.find(msg.id()->string_view());
        if (it != core.tagged_frames.end())
        {
            load(it->second, msg.tag()->string_view());
        }
    }

    void handle(Core& core, const nil::service::ID& id, const UniqueFrameSubscribe& msg)
    {
        const auto it = core.unique_frames.find(msg.id()->string_view());
        if (it != core.unique_frames.end())
        {
            subscribe(it->second, id);
        }
    }

    void handle(Core& core, const nil::service::ID& id, const TaggedFrameSubscribe& msg)
    {
        const auto it = core.tagged_frames.find(msg.id()->string_view());
        if (it != core.tagged_frames.end())
        {
            subscribe(it->second, msg.tag()->string_view(), id);
        }
    }

    void handle(Core& core, const nil::service::ID& id, const UniqueFrameUnsubscribe& msg)
    {
        const auto it = core.unique_frames.find(msg.id()->string_view());
        if (it != core.unique_frames.end())
        {
            unsubscribe(it->second, id);
        }
    }

    void handle(Core& core, const nil::service::ID& id, const TaggedFrameUnsubscribe& msg)
    {
        const auto it = core.tagged_frames.find(msg.id()->string_view());
        if (it != core.tagged_frames.end())
        {
            unsubscribe(it->second, msg.tag()->string_view(), id);
        }
    }

    void handle(Core& core, const nil::service::ID& id, const UniqueValueRequest& request)
    {
        const auto it = core.unique_frames.find(request.id()->string_view());
        if (it != core.unique_frames.end())
        {
            flatbuffers::FlatBufferBuilder builder;

            std::vector<ValueT> values;
            std::vector<flatbuffers::Offset<Value>> value_offsets;

            auto& [frame_id, frame] = *it;
            values.reserve(frame.values.size());
            value_offsets.reserve(frame.values.size());
            for (const auto& [value_id, value] : frame.values)
            {
                auto& new_value = values.emplace_back();
                new_value.id = value_id;
                new_value.value = value.accessor->get();
                value_offsets.emplace_back(CreateValue(
                    builder,
                    builder.CreateString(new_value.id),
                    builder.CreateVector(new_value.value)
                ));
            }

            builder.Finish(CreateUniqueValueResponse(
                builder,
                builder.CreateString(request.id()),
                builder.CreateVector(value_offsets)
            ));

            const auto header = MessageType_Server_Unique_Value_Response;
            auto payload = nil::service::concat(header, builder);
            core.msg_service->send(id, std::move(payload));
        }
    }

    void handle(Core& core, const nil::service::ID& id, const TaggedValueRequest& request)
    {
        const auto it = core.tagged_frames.find(request.id()->string_view());
        if (it != core.tagged_frames.end())
        {
            flatbuffers::FlatBufferBuilder builder;

            std::vector<ValueT> values;
            std::vector<flatbuffers::Offset<Value>> value_offsets;

            auto& [frame_id, frame] = *it;
            values.reserve(frame.values.size());
            value_offsets.reserve(frame.values.size());
            for (const auto& [value_id, value] : frame.values)
            {
                auto& new_value = values.emplace_back();
                new_value.id = value_id;
                new_value.value = value.accessor->get(request.tag()->string_view());
                value_offsets.emplace_back(CreateValue(
                    builder,
                    builder.CreateString(new_value.id),
                    builder.CreateVector(new_value.value)
                ));
            }

            builder.Finish(CreateTaggedValueResponse(
                builder,
                builder.CreateString(request.id()),
                builder.CreateString(request.tag()),
                builder.CreateVector(value_offsets)
            ));

            const auto header = MessageType_Server_Tagged_Value_Response;
            auto payload = nil::service::concat(header, builder);
            core.msg_service->send(id, std::move(payload));
        }
    }

    void handle(Core& core, const nil::service::ID& id, const UniqueSignalRequest& request)
    {
        const auto it = core.unique_frames.find(request.id()->string_view());
        if (it != core.unique_frames.end())
        {
            auto& [frame_id, frame] = *it;

            flatbuffers::FlatBufferBuilder builder;

            std::vector<SignalT> signals;
            std::vector<flatbuffers::Offset<Signal>> signal_offsets;

            signals.reserve(frame.signals.size());
            signal_offsets.reserve(frame.signals.size());
            for (const auto& [signal_id, signal] : frame.signals)
            {
                auto& new_signal = signals.emplace_back();
                new_signal.id = signal_id;

                signal_offsets.emplace_back(
                    CreateSignal(builder, builder.CreateString(new_signal.id))
                );
            }

            builder.Finish(CreateUniqueSignalResponse(
                builder,
                builder.CreateString(request.id()),
                builder.CreateVector(signal_offsets)
            ));

            const auto header = MessageType_Server_Unique_Signal_Response;
            auto payload = nil::service::concat(header, builder);
            core.msg_service->send(id, std::move(payload));
        }
    }

    void handle(Core& core, const nil::service::ID& id, const TaggedSignalRequest& request)
    {
        const auto it = core.tagged_frames.find(request.id()->string_view());
        if (it != core.tagged_frames.end())
        {
            auto& [frame_id, frame] = *it;

            flatbuffers::FlatBufferBuilder builder;

            std::vector<SignalT> signals;
            std::vector<flatbuffers::Offset<Signal>> signal_offsets;

            signals.reserve(frame.signals.size());
            signal_offsets.reserve(frame.signals.size());
            for (const auto& [signal_id, signal] : frame.signals)
            {
                auto& new_signal = signals.emplace_back();
                new_signal.id = signal_id;

                signal_offsets.emplace_back(
                    CreateSignal(builder, builder.CreateString(new_signal.id))
                );
            }

            builder.Finish(CreateTaggedSignalResponse(
                builder,
                builder.CreateString(request.id()),
                builder.CreateString(request.tag()),
                builder.CreateVector(signal_offsets)
            ));

            const auto header = MessageType_Server_Tagged_Signal_Response;
            auto payload = nil::service::concat(header, builder);
            core.msg_service->send(id, std::move(payload));
        }
    }

    void handle(Core& core, const nil::service::ID& id, const TaggedValueUpdate& request)
    {
        auto it = core.tagged_frames.find(request.id()->string_view());
        if (it != core.tagged_frames.end())
        {
            auto v_it = it->second.values.find(request.value()->id()->string_view());
            if (v_it != it->second.values.end())
            {
                value_set(v_it->second, *request.value(), request.tag()->string_view(), id);
            }
        }
    }

    void handle(Core& core, const nil::service::ID& id, const UniqueValueUpdate& request)
    {
        auto it = core.unique_frames.find(request.id()->string_view());
        if (it != core.unique_frames.end())
        {
            auto v_it = it->second.values.find(request.value()->id()->string_view());
            if (v_it != it->second.values.end())
            {
                value_set(v_it->second, *request.value(), id);
            }
        }
    }

    void handle(Core& core, const nil::service::ID& /* id */, const UniqueSignalNotify& request)
    {
        auto it = core.unique_frames.find(request.frame_id()->string_view());
        if (it != core.unique_frames.end())
        {
            auto s_it = it->second.signals.find(request.signal_id()->string_view());
            if (s_it != it->second.signals.end())
            {
                invoke(s_it->second, request);
            }
        }
    }

    void handle(Core& core, const nil::service::ID& /* id */, const TaggedSignalNotify& request)
    {
        auto it = core.tagged_frames.find(request.frame_id()->string_view());
        if (it != core.tagged_frames.end())
        {
            auto s_it = it->second.signals.find(request.signal_id()->string_view());
            if (s_it != it->second.signals.end())
            {
                invoke(s_it->second, request, request.tag()->string_view());
            }
        }
    }

    template <xalt::literal type>
    auto handle_frame_cache(Core* core)
    {
        return [core](const nil::service::ID& /* id */, const void* data, std::uint64_t size)
        {
            if (!core->cache_location.has_value())
            {
                return;
            }

            flatbuffers::Verifier verifier(static_cast<const std::uint8_t*>(data), size);
            if (!verifier.VerifyBuffer<FrameCache>())
            {
                return;
            }

            const auto* message = flatbuffers::GetRoot<FrameCache>(data);
            if (message == nullptr)
            {
                return;
            }

            const auto frame_id = message->id()->string_view();
            flatbuffers::FlatBufferBuilder builder;

            std::vector<flatbuffers::Offset<Alias>> alias_offsets;
            const auto* groups = message->groups();
            if (groups != nullptr)
            {
                alias_offsets.reserve(groups->size());
                for (const auto* group_name : *groups)
                {
                    const auto group_sv = group_name->string_view();
                    const auto it = core->groups.find(group_sv);
                    if (it == core->groups.end())
                    {
                        return;
                    }
                    alias_offsets.emplace_back(CreateAlias(
                        builder,
                        builder.CreateString(group_sv),
                        builder.CreateString(it->second.string())
                    ));
                }
            }

            auto file_info_offset_for
                = [&builder](const FileInfo* info) -> flatbuffers::Offset<FileInfo>
            {
                const auto* metadata = info->metadata();
                const auto metadata_offset
                    = builder.CreateVector(metadata->data(), metadata->size());
                return CreateFileInfo(
                    builder,
                    builder.CreateString(info->group()->string_view()),
                    builder.CreateString(info->path()->string_view()),
                    metadata_offset
                );
            };

            std::vector<flatbuffers::Offset<FileInfo>> file_offsets;
            const auto* files = message->files();
            if (files != nullptr)
            {
                file_offsets.reserve(files->size());
                for (const auto* info : *files)
                {
                    file_offsets.emplace_back(file_info_offset_for(info));
                }
            }

            std::vector<flatbuffers::Offset<flatbuffers::String>> group_offsets;
            const auto* groups_offset_source = message->groups();
            if (groups_offset_source != nullptr)
            {
                group_offsets.reserve(groups_offset_source->size());
                for (const auto* group_name : *groups_offset_source)
                {
                    group_offsets.emplace_back(builder.CreateString(group_name->string_view()));
                }
            }

            std::vector<flatbuffers::Offset<Option>> option_offsets;
            const auto* option_source = message->options();
            if (option_source != nullptr)
            {
                option_offsets.reserve(option_source->size());
                for (const auto* option : *option_source)
                {
                    option_offsets.emplace_back(CreateOption(
                        builder,
                        builder.CreateString(option->key()->string_view()),
                        builder.CreateString(option->value()->string_view())
                    ));
                }
            }

            const auto cache_offset = CreateFrameCache(
                builder,
                builder.CreateString(message->id()->string_view()),
                builder.CreateVector(file_offsets),
                builder.CreateVector(group_offsets),
                builder.CreateVector(option_offsets),
                builder.CreateString(message->content()->string_view())
            );
            const auto groups_offset = builder.CreateVector(alias_offsets);
            const auto save_offset = CreateFrameCacheSave(builder, cache_offset, groups_offset);
            builder.Finish(save_offset);

            std::ofstream f(
                *core->cache_location / xalt::literal_sv<type> / frame_id,
                std::ios::binary | std::ios::out
            );
            f.write(
                reinterpret_cast<const char*>(builder.GetBufferPointer()), // NOLINT
                std::int64_t(builder.GetSize())
            );
        };
    }

    template <typename T>
    auto handle(Core* core)
    {
        return [core](const auto& id, const void* data, std::uint64_t /* size */)
        {
            const auto message = flatbuffers::GetRoot<T>(data);
            if (message != nullptr)
            {
                handle(*core, id, *message);
            }
        };
    }

    void on_message(nil::service::ICallbackService& service, Core* ptr)
    {
        using nil::service::map;
        using nil::service::mapping;
        service.on_message(
            // clang-format off
            map(mapping(MessageType_Client_Unique_FrameInfo_Request, handle<UniqueFrameInfoRequest>(ptr)),
                mapping(MessageType_Client_Tagged_FrameInfo_Request, handle<TaggedFrameInfoRequest>(ptr)),
                mapping(MessageType_Client_File_Request, handle<FileRequest>(ptr)),
                mapping(MessageType_Client_Unique_Frame_Loaded, handle<UniqueFrameLoaded>(ptr)),
                mapping(MessageType_Client_Tagged_Frame_Loaded, handle<TaggedFrameLoaded>(ptr)),
                mapping(MessageType_Client_Unique_Frame_Subscribe, handle<UniqueFrameSubscribe>(ptr)),
                mapping(MessageType_Client_Tagged_Frame_Subscribe, handle<TaggedFrameSubscribe>(ptr)),
                mapping(MessageType_Client_Unique_Frame_Unsubscribe, handle<UniqueFrameUnsubscribe>(ptr)),
                mapping(MessageType_Client_Tagged_Frame_Unsubscribe, handle<TaggedFrameUnsubscribe>(ptr)),
                mapping(MessageType_Client_Unique_Value_Request, handle<UniqueValueRequest>(ptr)),
                mapping(MessageType_Client_Tagged_Value_Request, handle<TaggedValueRequest>(ptr)),
                mapping(MessageType_Client_Unique_Signal_Request, handle<UniqueSignalRequest>(ptr)),
                mapping(MessageType_Client_Tagged_Signal_Request, handle<TaggedSignalRequest>(ptr)),
                mapping(MessageType_Client_Unique_FrameCache, handle_frame_cache<"unique">(ptr)),
                mapping(MessageType_Client_Tagged_FrameCache, handle_frame_cache<"tagged">(ptr)),
                mapping(MessageType_Tagged_Value_Update, handle<TaggedValueUpdate>(ptr)),
                mapping(MessageType_Unique_Value_Update, handle<UniqueValueUpdate>(ptr)),
                mapping(MessageType_Client_Tagged_Signal_Notify, handle<TaggedSignalNotify>(ptr)),
                mapping(MessageType_Client_Unique_Signal_Notify, handle<UniqueSignalNotify>(ptr)))
            // clang-format on
        );
    }

    void on_disconnect(nil::service::ICallbackService& service, Core* ptr)
    {
        service.on_disconnect(
            [ptr](const auto& id)
            {
                for (auto& pair : ptr->unique_frames)
                {
                    unsubscribe(pair.second, id);
                }
                for (auto& pair : ptr->tagged_frames)
                {
                    unsubscribe(pair.second, id);
                }
            }
        );
    }
}

namespace nil::xit
{
    core_ptr make_core(
        nil::service::IRunnableService& run_service,
        nil::service::IEventService& event_service
    )
    {
        return {create_core(run_service, event_service), &destroy_core};
    }

    core_ptr make_core(nil::service::IStandaloneService& service)
    {
        return make_core(service, service);
    }

    Core* create_core(
        nil::service::IRunnableService& run_service,
        nil::service::IEventService& event_service
    )
    {
        Core* ptr = new Core(
            &run_service,
            &event_service,
            {},
            nil::xalt::transparent_umap<std::filesystem::path>(),
            nil::xalt::transparent_umap<unique::Frame>(),
            nil::xalt::transparent_umap<tagged::Frame>()
        );
        fbs::on_message(event_service, ptr);
        fbs::on_disconnect(event_service, ptr);
        event_service.on_ready(
            [ptr]()
            {
                if (ptr->cache_location.has_value())
                {
                    std::filesystem::create_directories(*ptr->cache_location / "unique");
                    std::filesystem::create_directories(*ptr->cache_location / "tagged");
                }
            }
        );
        return ptr;
    }

    Core* create_core(nil::service::IStandaloneService& service)
    {
        return create_core(service, service);
    }

    void destroy_core(Core* core)
    {
        std::default_delete<Core>()(core);
    }

    void set_cache_directory(Core& core, std::optional<std::filesystem::path> tmp_path) // NOLINT
    {
        core.cache_location = std::move(tmp_path);
    }

    void set_groups(Core& core, nil::xalt::transparent_umap<std::filesystem::path> groups)
    {
        core.groups = std::move(groups);
    }

    const nil::xalt::transparent_umap<std::filesystem::path>& get_groups(const Core& core)
    {
        return core.groups;
    }
}
