#include "cache_utils.hpp"
#include "codec.hpp" // IWYU pragma: keep
#include "structs.hpp"
#include "tagged/utils.hpp" // IWYU pragma: keep
#include "unique/utils.hpp" // IWYU pragma: keep

#include <nil/xalt/transparent_stl.hpp>

#include <nil/service/codec.hpp>
#include <nil/service/concat.hpp>
#include <nil/service/consume.hpp>
#include <nil/service/map.hpp>
#include <nil/service/structs.hpp>

#include <flatbuffers/buffer.h>
#include <flatbuffers/flatbuffer_builder.h>
#include <flatbuffers/flatbuffers.h>

#include <filesystem>
#include <optional>
#include <system_error>

namespace nil::xit::fbs
{
    namespace
    {
        template <typename Frame, typename ValueGetter>
        std::vector<flatbuffers::Offset<Value>> build_value_offsets(
            flatbuffers::FlatBufferBuilder& builder,
            const Frame& frame,
            ValueGetter get_value
        )
        {
            std::vector<ValueT> values;
            std::vector<flatbuffers::Offset<Value>> value_offsets;
            values.reserve(frame.values.size());
            value_offsets.reserve(frame.values.size());
            for (const auto& [value_id, value] : frame.values)
            {
                auto& new_value = values.emplace_back();
                new_value.id = value_id;
                new_value.value = get_value(value);
                value_offsets.emplace_back(CreateValue(
                    builder,
                    builder.CreateString(new_value.id),
                    builder.CreateVector(new_value.value)
                ));
            }

            return value_offsets;
        }

        template <typename Frame>
        std::vector<flatbuffers::Offset<Signal>> build_signal_offsets(
            flatbuffers::FlatBufferBuilder& builder,
            const Frame& frame
        )
        {
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

            return signal_offsets;
        }

        void send_message(
            Core& core,
            const nil::service::ID& id,
            MessageType header,
            flatbuffers::FlatBufferBuilder& builder
        )
        {
            auto payload = nil::service::concat(header, builder);
            core.msg_service->send(id, std::move(payload));
        }
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

            if (auto cache_content = load_valid_cache_content(
                    core,
                    CacheType::unique,
                    frame_id,
                    frame.options,
                    *frame.file_info
                ))
            {
                const auto* save = flatbuffers::GetRoot<FrameCacheSave>(cache_content->data());
                const auto* cache = save->cache();
                const auto* cache_options = cache->options();
                flatbuffers::FlatBufferBuilder builder;
                auto option_offsets = build_option_offsets(builder, cache_options);
                builder.Finish(CreateUniqueFrameInfoResponse(
                    builder,
                    builder.CreateString(cache->id()),
                    builder.CreateString(frame.file_info->group.c_str()),
                    builder.CreateString(frame.file_info->path.c_str()),
                    builder.CreateVector(option_offsets),
                    builder.CreateString(cache->content())
                ));
                send_message(core, id, MessageType_Server_Unique_FrameInfo_Response, builder);
                return;
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

            send_message(core, id, MessageType_Server_Unique_FrameInfo_Response, builder);
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

            if (auto cache_content = load_valid_cache_content(
                    core,
                    CacheType::tagged,
                    frame_id,
                    frame.options,
                    *frame.file_info
                ))
            {
                const auto tag = message.tag()->string_view();
                const auto* save = flatbuffers::GetRoot<FrameCacheSave>(cache_content->data());
                const auto* cache = save->cache();
                const auto* cache_options = cache->options();
                flatbuffers::FlatBufferBuilder builder;
                auto option_offsets = build_option_offsets(builder, cache_options);
                builder.Finish(CreateTaggedFrameInfoResponse(
                    builder,
                    builder.CreateString(cache->id()),
                    builder.CreateString(tag),
                    builder.CreateString(frame.file_info->group.c_str()),
                    builder.CreateString(frame.file_info->path.c_str()),
                    builder.CreateVector(option_offsets),
                    builder.CreateString(cache->content())
                ));
                send_message(core, id, MessageType_Server_Tagged_FrameInfo_Response, builder);
                return;
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

            send_message(core, id, MessageType_Server_Tagged_FrameInfo_Response, builder);
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
        std::error_code fs_error;
        if (!std::filesystem::exists(target, fs_error) || fs_error)
        {
            return;
        }

        const auto target_time_point = std::filesystem::last_write_time(target, fs_error);
        if (fs_error)
        {
            return;
        }

        const auto content = load_file(target);
        const auto target_time = target_time_point.time_since_epoch().count();
        const auto metadata = nil::service::concat(target_time);

        flatbuffers::FlatBufferBuilder builder;
        builder.Finish(CreateFileResponse(
            builder,
            builder.CreateString(group),
            builder.CreateString(path),
            builder.CreateString(content),
            builder.CreateVector(metadata)
        ));

        send_message(core, id, MessageType_Server_File_Response, builder);
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

            auto& frame = it->second;
            auto value_offsets = build_value_offsets(
                builder,
                frame,
                [](const auto& value) { return value.accessor->get(); }
            );

            builder.Finish(CreateUniqueValueResponse(
                builder,
                builder.CreateString(request.id()),
                builder.CreateVector(value_offsets)
            ));

            send_message(core, id, MessageType_Server_Unique_Value_Response, builder);
        }
    }

    void handle(Core& core, const nil::service::ID& id, const TaggedValueRequest& request)
    {
        const auto it = core.tagged_frames.find(request.id()->string_view());
        if (it != core.tagged_frames.end())
        {
            flatbuffers::FlatBufferBuilder builder;

            auto& frame = it->second;
            const auto tag = request.tag()->string_view();
            auto value_offsets = build_value_offsets(
                builder,
                frame,
                [tag](const auto& value) { return value.accessor->get(tag); }
            );

            builder.Finish(CreateTaggedValueResponse(
                builder,
                builder.CreateString(request.id()),
                builder.CreateString(request.tag()),
                builder.CreateVector(value_offsets)
            ));

            send_message(core, id, MessageType_Server_Tagged_Value_Response, builder);
        }
    }

    void handle(Core& core, const nil::service::ID& id, const UniqueSignalRequest& request)
    {
        const auto it = core.unique_frames.find(request.id()->string_view());
        if (it != core.unique_frames.end())
        {
            auto& frame = it->second;

            flatbuffers::FlatBufferBuilder builder;
            auto signal_offsets = build_signal_offsets(builder, frame);

            builder.Finish(CreateUniqueSignalResponse(
                builder,
                builder.CreateString(request.id()),
                builder.CreateVector(signal_offsets)
            ));

            send_message(core, id, MessageType_Server_Unique_Signal_Response, builder);
        }
    }

    void handle(Core& core, const nil::service::ID& id, const TaggedSignalRequest& request)
    {
        const auto it = core.tagged_frames.find(request.id()->string_view());
        if (it != core.tagged_frames.end())
        {
            auto& frame = it->second;

            flatbuffers::FlatBufferBuilder builder;
            auto signal_offsets = build_signal_offsets(builder, frame);

            builder.Finish(CreateTaggedSignalResponse(
                builder,
                builder.CreateString(request.id()),
                builder.CreateString(request.tag()),
                builder.CreateVector(signal_offsets)
            ));

            send_message(core, id, MessageType_Server_Tagged_Signal_Response, builder);
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

    template <CacheType type>
    auto handle_frame_cache(Core* core)
    {
        return [core](const nil::service::ID& /* id */, const void* data, std::uint64_t size)
        { handle_frame_cache_message(*core, data, size, type); };
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
                mapping(MessageType_Client_Unique_FrameCache, handle_frame_cache<CacheType::unique>(ptr)),
                mapping(MessageType_Client_Tagged_FrameCache, handle_frame_cache<CacheType::tagged>(ptr)),
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
                if (!ptr->cache_location.has_value())
                {
                    return;
                }

                const auto& loc = *ptr->cache_location;
                std::filesystem::create_directories(loc / fbs::cache_dir(fbs::CacheType::unique));
                std::filesystem::create_directories(loc / fbs::cache_dir(fbs::CacheType::tagged));
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
