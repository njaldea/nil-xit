#include "ValueType.hpp"

#include <nil/xit.h>

#include <nil/xit.hpp>

#include <nil/xalt/raii.hpp>

#include <filesystem>
#include <memory>
#include <string_view>

namespace
{
    nil::xit::Core* from_c(nil_xit_core core)
    {
        return static_cast<nil::xit::Core*>(core.handle);
    }

    nil::xit::unique::Frame* from_c(nil_xit_unique_frame frame)
    {
        return static_cast<nil::xit::unique::Frame*>(frame.handle);
    }

    nil::service::IWebService* from_c(nil_service_web service)
    {
        return static_cast<nil::service::IWebService*>(service.handle);
    }

    nil_xit_core to_c(nil::xit::Core* core)
    {
        return {.handle = core};
    }

    nil_xit_unique_frame to_c(nil::xit::unique::Frame& frame)
    {
        return {.handle = &frame};
    }
}

extern "C"
{
    nil_xit_core nil_xit_core_create(
        nil_service_runnable run_service,
        nil_service_event event_service
    )
    {
        auto* runnable = static_cast<nil::service::IRunnableService*>(run_service.handle);
        auto* event = static_cast<nil::service::IEventService*>(event_service.handle);
        return to_c(nil::xit::create_core(*runnable, *event));
    }

    nil_xit_core nil_xit_core_create_from_standalone(nil_service_standalone service)
    {
        auto* standalone = static_cast<nil::service::IStandaloneService*>(service.handle);
        return to_c(nil::xit::create_core(*standalone));
    }

    void nil_xit_setup_server(nil_service_web service, const char** asset_paths, size_t count)
    {
        std::vector<std::filesystem::path> paths;
        paths.reserve(count);
        for (size_t i = 0; i < count; ++i)
        {
            paths.emplace_back(asset_paths[i]);
        }
        nil::xit::setup_server(*from_c(service), paths);
    }

    void nil_xit_set_cache_directory(nil_xit_core core, const char* tmp_path)
    {
        nil::xit::set_cache_directory(*from_c(core), std::filesystem::path(tmp_path));
    }

    void nil_xit_set_groups(nil_xit_core core, const nil_xit_group_entry* groups, uint64_t size)
    {
        nil::xalt::transparent_umap<std::filesystem::path> mapped_groups;
        for (uint64_t i = 0; i < size; ++i)
        {
            mapped_groups.emplace(groups[i].group, std::filesystem::path(groups[i].path));
        }

        nil::xit::set_groups(*from_c(core), std::move(mapped_groups));
    }

    void nil_xit_core_destroy(nil_xit_core core)
    {
        nil::xit::destroy_core(from_c(core));
    }

    nil_xit_unique_frame nil_xit_core_add_unique_frame(
        nil_xit_core core,
        const char* id,
        const char* path
    )
    {
        if (path == nullptr)
        {
            return to_c(nil::xit::add_unique_frame(*from_c(core), id));
        }

        return to_c(nil::xit::add_unique_frame(*from_c(core), id, path));
    }

    nil_xit_unique_frame nil_xit_core_add_unique_frame_with_path(
        nil_xit_core core,
        const char* id,
        const char* path
    )
    {
        return to_c(nil::xit::add_unique_frame(*from_c(core), id, path));
    }

    void nil_xit_unique_frame_on_load(
        nil_xit_unique_frame frame,
        nil_xit_unique_callback_info callback
    )
    {
        auto holder = std::make_shared<nil::xalt::raii<void>>(callback.context, callback.cleanup);

        nil::xit::unique::on_load(
            *from_c(frame),
            [exec = callback.exec, holder]() { exec(holder->object); }
        );
    }

    void nil_xit_unique_frame_on_sub(
        nil_xit_unique_frame frame,
        nil_xit_unique_on_sub_info callback
    )
    {
        auto holder = std::make_shared<nil::xalt::raii<void>>(callback.context, callback.cleanup);

        nil::xit::unique::on_sub(
            *from_c(frame),
            [exec = callback.exec, holder](std::size_t count)
            { exec(static_cast<std::uint64_t>(count), holder->object); }
        );
    }

    nil_xit_unique_frame_value nil_xit_unique_frame_add_value(
        nil_xit_unique_frame frame,
        const char* id,
        nil_xit_unique_value_accessor accessor
    )
    {
        auto& value = nil::xit::unique::add_value(
            *from_c(frame),
            std::string(id),
            std::make_unique<nil::xit::c::UniqueAccessor>(accessor)
        );

        return {.handle = &value};
    }

    void nil_xit_unique_value_post(
        nil_xit_unique_frame_value value,
        const void* new_data,
        uint64_t new_data_size
    )
    {
        auto* value_handle
            = static_cast<nil::xit::unique::Value<std::vector<std::uint8_t>>*>(value.handle);

        const auto* start = static_cast<const std::uint8_t*>(new_data);
        nil::xit::unique::post(*value_handle, {start, start + new_data_size});
    }

    void nil_xit_unique_frame_add_signal(
        nil_xit_unique_frame frame,
        const char* id,
        nil_xit_unique_callback_info callback
    )
    {
        // Hold context and cleanup for the lifetime of the callback
        auto holder = std::make_shared<nil::xalt::raii<void>>(callback.context, callback.cleanup);

        // The callback signature for unique frames is void().
        nil::xit::unique::add_signal(
            *from_c(frame),
            std::string(id),
            [exec = callback.exec, holder]() { exec(holder->object); }
        );
    }

    nil_xit_tagged_frame nil_xit_core_add_tagged_frame(
        nil_xit_core core,
        const char* id,
        const char* path
    )
    {
        if (path == nullptr)
        {
            return {.handle = &nil::xit::add_tagged_frame(*from_c(core), id)};
        }
        return {.handle = &nil::xit::add_tagged_frame(*from_c(core), id, path)};
    }

    nil_xit_tagged_frame nil_xit_core_add_tagged_frame_with_path(
        nil_xit_core core,
        const char* id,
        const char* path
    )
    {
        return {.handle = &nil::xit::add_tagged_frame(*from_c(core), id, path)};
    }

    void nil_xit_tagged_frame_on_load(
        nil_xit_tagged_frame frame,
        nil_xit_tagged_callback_info callback
    )
    {
        auto holder = std::make_shared<nil::xalt::raii<void>>(callback.context, callback.cleanup);
        nil::xit::tagged::on_load(
            *static_cast<nil::xit::tagged::Frame*>(frame.handle),
            [exec = callback.exec, holder](std::string_view tag)
            { exec(tag.data(), holder->object); }
        );
    }

    void nil_xit_tagged_frame_on_sub(
        nil_xit_tagged_frame frame,
        nil_xit_tagged_on_sub_info callback
    )
    {
        auto holder = std::make_shared<nil::xalt::raii<void>>(callback.context, callback.cleanup);
        nil::xit::tagged::on_sub(
            *static_cast<nil::xit::tagged::Frame*>(frame.handle),
            [exec = callback.exec, holder](std::string_view tag, std::size_t count)
            { exec(tag.data(), static_cast<uint64_t>(count), holder->object); }
        );
    }

    nil_xit_tagged_frame_value nil_xit_tagged_frame_add_value(
        nil_xit_tagged_frame frame,
        const char* id,
        nil_xit_tagged_value_accessor accessor
    )
    {
        auto& value = nil::xit::tagged::add_value(
            *static_cast<nil::xit::tagged::Frame*>(frame.handle),
            std::string(id),
            std::make_unique<nil::xit::c::TaggedAccessor>(accessor)
        );
        return {.handle = &value};
    }

    void nil_xit_tagged_frame_add_signal(
        nil_xit_tagged_frame frame,
        const char* id,
        nil_xit_tagged_callback_info callback
    )
    {
        auto holder = std::make_shared<nil::xalt::raii<void>>(callback.context, callback.cleanup);
        nil::xit::tagged::add_signal(
            *static_cast<nil::xit::tagged::Frame*>(frame.handle),
            std::string(id),
            [exec = callback.exec, holder](std::string_view tag)
            { exec(tag.data(), holder->object); }
        );
    }

    void nil_xit_tagged_value_post(
        nil_xit_tagged_frame_value value,
        const char* tag,
        const void* new_data,
        uint64_t new_data_size
    )
    {
        auto* value_handle
            = static_cast<nil::xit::tagged::Value<std::vector<std::uint8_t>>*>(value.handle);

        const auto* start = static_cast<const std::uint8_t*>(new_data);
        nil::xit::tagged::post(*value_handle, std::string(tag), {start, start + new_data_size});
    }
}
