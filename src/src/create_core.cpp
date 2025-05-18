#include "codec.hpp"
#include "messages/message.fbs.h"
#include "structs.hpp"
#include "tagged/utils.hpp" // IWYU pragma: keep
#include "unique/utils.hpp" // IWYU pragma: keep
#include "utils.hpp"        // IWYU pragma: keep

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
    std::string load_file(const std::filesystem::path& path)
    {
        std::ifstream f(path, std::ios::binary | std::ios::in);
        return {std::istream_iterator<char>{f >> std::noskipws}, std::istream_iterator<char>{}};
    }

    bool validate_cache(
        Core& core,
        const nil::service::ID& id,
        const std::filesystem::path& path,
        std::optional<std::string_view> tag = {}
    )
    {
        if (std::filesystem::exists(path))
        {
            const auto content = load_file(path);
            const auto& cache = *flatbuffers::GetRoot<FrameCache>(content.data());

            for (const auto& ff : *cache.files())
            {
                const auto target = ff->target()->string_view();
                if (!std::filesystem::exists(target))
                {
                    return false;
                }

                const auto target_time
                    = std::filesystem::last_write_time(target).time_since_epoch().count();
                using target_time_t = std::decay_t<decltype(target_time)>;
                const auto* const metadata = ff->metadata();
                std::uint64_t size = metadata->size();
                if (sizeof(target_time) != size)
                {
                    return false;
                }
                const auto cached_target_time
                    = nil::service::codec<target_time_t>::deserialize(metadata->data(), size);
                if (cached_target_time != target_time)
                {
                    return false;
                }
            }

            if (tag.has_value())
            {
                flatbuffers::FlatBufferBuilder builder;
                builder.Finish(CreateTaggedFrameInfoResponse(
                    builder,
                    builder.CreateString(cache.id()),
                    builder.CreateString(tag.value()),
                    builder.CreateString(cache.content())
                ));

                const auto header = MessageType_Server_Tagged_FrameInfo_Content_Response;
                auto payload = nil::service::concat(header, builder);
                send(*core.service, id, std::move(payload));
            }
            else
            {
                flatbuffers::FlatBufferBuilder builder;
                builder.Finish(CreateUniqueFrameInfoResponse(
                    builder,
                    builder.CreateString(cache.id()),
                    builder.CreateString(cache.content())
                ));

                const auto header = MessageType_Server_Unique_FrameInfo_Content_Response;
                auto payload = nil::service::concat(header, builder);
                send(*core.service, id, std::move(payload));
            }
            return true;
        }
        return false;
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

            const auto is_cached = validate_cache(
                core,
                id,
                core.cache_location / "unique" / frame_id //
            );

            if (is_cached)
            {
                return;
            }

            if (const auto file_it = core.ui_directories.find(frame.file_info->group);
                file_it != core.ui_directories.end())
            {
                flatbuffers::FlatBufferBuilder builder;
                builder.Finish(CreateUniqueFrameInfoResponse(
                    builder,
                    builder.CreateString(frame_id),
                    builder.CreateString((file_it->second / frame.file_info->path).c_str())
                ));

                const auto header = MessageType_Server_Unique_FrameInfo_File_Response;
                auto payload = nil::service::concat(header, builder);
                send(*core.service, id, std::move(payload));
            }
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

            const auto is_cached = validate_cache(
                core,
                id,
                core.cache_location / "tagged" / frame_id,
                message.tag()->string_view()
            );

            if (is_cached)
            {
                return;
            }

            if (const auto file_it = core.ui_directories.find(frame.file_info->group);
                file_it != core.ui_directories.end())
            {
                flatbuffers::FlatBufferBuilder builder;
                builder.Finish(CreateTaggedFrameInfoResponse(
                    builder,
                    builder.CreateString(frame_id),
                    builder.CreateString(message.tag()),
                    builder.CreateString((file_it->second / frame.file_info->path).c_str())
                ));

                const auto header = MessageType_Server_Tagged_FrameInfo_File_Response;
                auto payload = nil::service::concat(header, builder);
                send(*core.service, id, std::move(payload));
            }
        }
    }

    void handle(Core& core, const nil::service::ID& id, const FileRequest& request)
    {
        const auto target = request.target()->string_view();
        const auto content = load_file(target);

        const auto target_time
            = std::filesystem::last_write_time(target).time_since_epoch().count();
        const auto metadata = nil::service::concat(target_time);

        flatbuffers::FlatBufferBuilder builder;
        builder.Finish(CreateFileResponse(
            builder,
            builder.CreateString(target),
            builder.CreateString(content),
            builder.CreateVector(metadata)
        ));

        auto header = MessageType_Server_File_Response;
        auto payload = nil::service::concat(header, builder);
        send(*core.service, id, std::move(payload));
    }

    void handle(Core& core, const nil::service::ID& id, const FileAliasRequest& /* request */)
    {
        flatbuffers::FlatBufferBuilder builder;
        std::vector<flatbuffers::Offset<FileAlias>> file_alias_offsets;
        file_alias_offsets.reserve(core.ui_directories.size());
        for (const auto& [key, alias] : core.ui_directories)
        {
            file_alias_offsets.emplace_back(CreateFileAlias(
                builder,
                builder.CreateString(key),
                builder.CreateString(alias.c_str())
            ));
        }
        builder.Finish(CreateFileAliasResponse(builder, builder.CreateVector(file_alias_offsets)));

        auto header = MessageType_Server_File_Alias_Response;
        auto payload = nil::service::concat(header, builder);
        send(*core.service, id, std::move(payload));
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
                std::visit(
                    [&new_value, &value_offsets, &builder](const auto& v)
                    {
                        value_offsets.emplace_back(CreateValue(
                            builder,
                            builder.CreateString(new_value.id),
                            new_value.value.type,
                            msg_set(v, new_value, builder).Union()
                        ));
                    },
                    value
                );
            }

            builder.Finish(CreateUniqueValueResponse(
                builder,
                builder.CreateString(request.id()),
                builder.CreateVector(value_offsets)
            ));

            const auto header = MessageType_Server_Unique_Value_Response;
            auto payload = nil::service::concat(header, builder);
            send(*core.service, id, std::move(payload));
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
                std::visit(
                    [&new_value, &value_offsets, &builder, &request](const auto& v)
                    {
                        value_offsets.emplace_back(CreateValue(
                            builder,
                            builder.CreateString(new_value.id),
                            new_value.value.type,
                            msg_set(v, new_value, builder, request.tag()->string_view()).Union()
                        ));
                    },
                    value
                );
            }

            builder.Finish(CreateTaggedValueResponse(
                builder,
                builder.CreateString(request.id()),
                builder.CreateString(request.tag()),
                builder.CreateVector(value_offsets)
            ));

            const auto header = MessageType_Server_Tagged_Value_Response;
            auto payload = nil::service::concat(header, builder);
            send(*core.service, id, std::move(payload));
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
                using nil::xit::utils::msg_set;
                std::visit(
                    [&new_signal, &signal_id](const auto& s)
                    {
                        new_signal.id = signal_id;
                        msg_set(s, new_signal);
                    },
                    signal
                );

                signal_offsets.emplace_back(
                    CreateSignal(builder, builder.CreateString(new_signal.id), new_signal.type)
                );
            }

            builder.Finish(CreateUniqueSignalResponse(
                builder,
                builder.CreateString(request.id()),
                builder.CreateVector(signal_offsets)
            ));

            const auto header = MessageType_Server_Unique_Signal_Response;
            auto payload = nil::service::concat(header, builder);
            send(*core.service, id, std::move(payload));
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
                using nil::xit::utils::msg_set;
                std::visit(
                    [&new_signal, &signal_id](const auto& s)
                    {
                        new_signal.id = signal_id;
                        msg_set(s, new_signal);
                    },
                    signal
                );

                signal_offsets.emplace_back(
                    CreateSignal(builder, builder.CreateString(new_signal.id), new_signal.type)
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
            send(*core.service, id, std::move(payload));
        }
    }

    void handle(Core& core, const nil::service::ID& id, const TaggedValueUpdate& request)
    {
        auto it = core.tagged_frames.find(request.id()->string_view());
        if (it != core.tagged_frames.end())
        {
            request.value();
            auto v_it = it->second.values.find(request.value()->id()->string_view());
            if (v_it != it->second.values.end())
            {
                std::visit(
                    [&request, &id](auto& v)
                    { value_set(v, *request.value(), request.tag()->string_view(), id); },
                    v_it->second
                );
            }
        }
    }

    void handle(Core& core, const nil::service::ID& id, const UniqueValueUpdate& request)
    {
        auto it = core.unique_frames.find(request.id()->string_view());
        if (it != core.unique_frames.end())
        {
            request.value();
            auto v_it = it->second.values.find(request.value()->id()->string_view());
            if (v_it != it->second.values.end())
            {
                std::visit(
                    [&request, &id](auto& v) { value_set(v, *request.value(), id); },
                    v_it->second
                );
            }
        }
    }

    void handle(Core& core, const nil::service::ID& /* id */, const UniqueSignalNotify& request)
    {
        auto it = core.unique_frames.find(request.frame_id()->string_view());
        if (it != core.unique_frames.end())
        {
            request.value();
            auto s_it = it->second.signals.find(request.signal_id()->string_view());
            if (s_it != it->second.signals.end())
            {
                auto& s = s_it->second;
                std::visit([&request](const auto& ss) { invoke(ss, request); }, s);
            }
        }
    }

    void handle(Core& core, const nil::service::ID& /* id */, const TaggedSignalNotify& request)
    {
        auto it = core.tagged_frames.find(request.frame_id()->string_view());
        if (it != core.tagged_frames.end())
        {
            request.value();
            auto s_it = it->second.signals.find(request.signal_id()->string_view());
            if (s_it != it->second.signals.end())
            {
                auto& s = s_it->second;
                std::visit(
                    [&request](const auto& ss)
                    { invoke(ss, request, request.tag()->string_view()); },
                    s
                );
            }
        }
    }

    auto handle_frame_cache(Core* core, std::string_view dir)
    {
        return [core, dir](const nil::service::ID& /* id */, const void* data, std::uint64_t size)
        {
            const auto* message = flatbuffers::GetRoot<FrameCache>(data);
            if (message != nullptr)
            {
                const auto frame_id = message->id()->string_view();
                std::ofstream f(
                    core->cache_location / dir / frame_id,
                    std::ios::binary | std::ios::out
                );
                f.write(static_cast<const char*>(data), std::int64_t(size));
            }
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

    void on_message(nil::service::P service, Core* ptr)
    {
        using nil::service::map;
        using nil::service::mapping;
        on_message(
            service,
            // clang-format off
            map(mapping(MessageType_Client_Unique_FrameInfo_Request, handle<UniqueFrameInfoRequest>(ptr)),
                mapping(MessageType_Client_Tagged_FrameInfo_Request, handle<TaggedFrameInfoRequest>(ptr)),
                mapping(MessageType_Client_File_Request, handle<FileRequest>(ptr)),
                mapping(MessageType_Client_File_Alias_Request, handle<FileAliasRequest>(ptr)),
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
                mapping(MessageType_Client_Unique_FrameCache, handle_frame_cache(ptr, "unique")),
                mapping(MessageType_Client_Tagged_FrameCache, handle_frame_cache(ptr, "tagged")),
                mapping(MessageType_Tagged_Value_Update, handle<TaggedValueUpdate>(ptr)),
                mapping(MessageType_Unique_Value_Update, handle<UniqueValueUpdate>(ptr)),
                mapping(MessageType_Client_Tagged_Signal_Notify, handle<TaggedSignalNotify>(ptr)),
                mapping(MessageType_Client_Unique_Signal_Notify, handle<UniqueSignalNotify>(ptr)))
            // clang-format on
        );
    }

    void on_disconnect(nil::service::P service, Core* ptr)
    {
        on_disconnect(
            service,
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
    C::operator Core&() const
    {
        return *ptr;
    }

    C make_core(nil::service::P service)
    {
        return {{create_core(service), &delete_core}};
    }

    Core* create_core(nil::service::P service)
    {
        Core* ptr = new Core(
            &static_cast<nil::service::MessagingService&>(service),
            std::filesystem::temp_directory_path() / "nil/xit",
            nil::xalt::transparent_umap<std::filesystem::path>(),
            nil::xalt::transparent_umap<unique::Frame>(),
            nil::xalt::transparent_umap<tagged::Frame>(),
            std::mutex()
        );
        fbs::on_message(service, ptr);
        fbs::on_disconnect(service, ptr);
        on_ready(
            service,
            [ptr]()
            {
                std::filesystem::create_directories(ptr->cache_location / "unique");
                std::filesystem::create_directories(ptr->cache_location / "tagged");
            }
        );
        return ptr;
    }

    void delete_core(Core* core)
    {
        std::default_delete<Core>()(core);
    }

    void set_cache_directory(Core& core, std::filesystem::path tmp_path) // NOLINT
    {
        tmp_path.append("nil/xit");
        core.cache_location = std::move(tmp_path);
    }

    void set_ui_directories(
        Core& core,
        nil::xalt::transparent_umap<std::filesystem::path> ui_directories
    )
    {
        core.ui_directories = std::move(ui_directories);
    }
}
