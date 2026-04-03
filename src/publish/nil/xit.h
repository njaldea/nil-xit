#pragma once

// NOLINTNEXTLINE(hicpp-deprecated-headers,modernize-deprecated-headers)
#include <stddef.h>

// NOLINTNEXTLINE(hicpp-deprecated-headers,modernize-deprecated-headers)
#include <stdint.h>

#include <nil/service.h>

#ifdef __cplusplus
extern "C"
{
#endif
    // Unless explicitly documented otherwise, API functions require valid non-null handles.

    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_core
    {
        void* handle;
    } nil_xit_core;

    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_unique_frame
    {
        void* handle;
    } nil_xit_unique_frame;

    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_unique_frame_value
    {
        void* handle;
        uint64_t (*encode_size)(const void* new_value);
        void (*encode)(const void* new_value, void* buffer);
    } nil_xit_unique_frame_value;

    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_bytes
    {
        void* data;
        uint64_t size;
    } nil_xit_bytes;

    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_callback_info
    {
        void (*exec)(void*);
        void* context;
        void (*cleanup)(void*);
    } nil_xit_callback_info;

    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_unique_on_sub_info
    {
        void (*exec)(uint64_t count, void*);
        void* context;
        void (*cleanup)(void*);
    } nil_xit_unique_on_sub_info;

    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_unique_value_accessor
    {
        uint64_t (*encode_size)(const void* ctx);
        void (*encode)(const void* ctx, void* buffer);
        void (*decode)(void* ctx, const void* buffer, uint64_t size);
        void* ctx;
        void (*cleanup)(void* ctx);
    } nil_xit_unique_value_accessor;

    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_group_entry
    {
        const char* group;
        const char* path;
    } nil_xit_group_entry;

    nil_xit_core nil_xit_core_create(
        nil_service_runnable run_service,
        nil_service_event event_service
    );

    nil_xit_core nil_xit_core_create_from_standalone(nil_service_standalone service);

    void nil_xit_setup_server(nil_service_web service, const char* asset_path);

    void nil_xit_set_cache_directory(nil_xit_core core, const char* tmp_path);

    void nil_xit_set_groups(nil_xit_core core, const nil_xit_group_entry* groups, uint64_t size);

    void nil_xit_core_destroy(nil_xit_core core);

    nil_xit_unique_frame nil_xit_core_add_unique_frame(
        nil_xit_core core,
        const char* id,
        const char* path
    );

    nil_xit_unique_frame nil_xit_core_add_unique_frame_with_path(
        nil_xit_core core,
        const char* id,
        const char* path
    );

    void nil_xit_unique_frame_on_load(nil_xit_unique_frame frame, nil_xit_callback_info callback);

    void nil_xit_unique_frame_on_sub(
        nil_xit_unique_frame frame,
        nil_xit_unique_on_sub_info callback
    );

    nil_xit_unique_frame_value nil_xit_unique_frame_add_value(
        nil_xit_unique_frame frame,
        const char* id,
        nil_xit_unique_value_accessor accessor
    );

    /**
     * Posts a new value for the given frame value.
     *
     * - `new_data` must match the type expected by the accessor's ctx.
     * - `new_data` is not owned by the library: it will not be freed, retained, or modified, and is
     * only used for initial serialization (encode) during this call.
     * - The accessor's encode function is called immediately to serialize `new_data`.
     * - Later, the accessor's decode function will be called with the encoded buffer.
     * - Only data present in the encoded buffer will be stored; any extra fields in `new_data` not
     * encoded will be lost.
     */
    void nil_xit_unique_value_post(nil_xit_unique_frame_value value, const void* new_data);
#ifdef __cplusplus
}
#endif
