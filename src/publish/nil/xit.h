#pragma once

// NOLINTNEXTLINE(hicpp-deprecated-headers,modernize-deprecated-headers)
#include <stddef.h>
// NOLINTNEXTLINE(hicpp-deprecated-headers,modernize-deprecated-headers)
#include <nil/service.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @file xit.h
     * @brief C API for nil-xit core and frame management.
     *
     * Unless explicitly documented otherwise, API functions require valid non-null handles.
     * All handles are opaque and must be created/destroyed via the API.
     */

    /**
     * @brief Opaque handle to a nil-xit core instance.
     *
     * Use only with nil_xit_core_* API functions.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_core
    {
        void* handle;
    } nil_xit_core;

    /**
     * @brief Opaque handle to a unique frame.
     *
     * Use only with nil_xit_unique_frame_* API functions.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_unique_frame
    {
        void* handle;
    } nil_xit_unique_frame;

    /**
     * @brief Opaque handle to a unique frame value.
     *
     * Provided by the library. Contains encode helpers for posting values.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_unique_frame_value
    {
        void* handle;
    } nil_xit_unique_frame_value;

    /**
     * @brief Generic callback info for events.
     *
     * Used for registering callbacks with context and cleanup.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_callback_info
    {
        void (*exec)(void*);    ///< Callback function.
        void* context;          ///< User context pointer.
        void (*cleanup)(void*); ///< Cleanup function for context.
    } nil_xit_callback_info;

    /**
     * @brief Callback info for unique frame events.
     *
     * Used for registering callbacks with context and cleanup (unique frame variant).
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_unique_callback_info
    {
        void (*exec)(void*);    ///< Callback function.
        void* context;          ///< User context pointer.
        void (*cleanup)(void*); ///< Cleanup function for context.
    } nil_xit_unique_callback_info;

    /**
     * @brief Callback info for unique frame subscription events.
     *
     * Used for on_sub event registration.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_unique_on_sub_info
    {
        void (*exec)(uint64_t count, void*); ///< Callback with count and context.
        void* context;                       ///< User context pointer.
        void (*cleanup)(void*);              ///< Cleanup function for context.
    } nil_xit_unique_on_sub_info;

    /**
     * @brief Value accessor for unique frame values.
     *
     * User-provided struct for encoding/decoding value types.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_unique_value_accessor
    {
        uint64_t (*encode_size)(const void* ctx);      ///< Returns encoded size for a value.
        void (*encode)(const void* ctx, void* buffer); ///< Encodes value into buffer.
        void (*decode)(
            void* ctx,
            const void* buffer,
            uint64_t size
        );                          ///< Decodes buffer into value.
        void* ctx;                  ///< User context for encode/decode.
        void (*cleanup)(void* ctx); ///< Cleanup function for context.
    } nil_xit_unique_value_accessor;

    /**
     * @brief Value accessor for unique frame values.
     *
     * User-provided struct for encoding/decoding value types.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_tagged_value_accessor
    {
        uint64_t (*encode_size)(
            const char* tag,
            const void* ctx
        ); ///< Returns encoded size for a value.
        void (*encode)(
            const char* tag,
            const void* ctx,
            void* buffer
        ); ///< Encodes value into buffer.
        void (*decode)(
            const char* tag,
            void* ctx,
            const void* buffer,
            uint64_t size
        );                          ///< Decodes buffer into value.
        void* ctx;                  ///< User context for encode/decode.
        void (*cleanup)(void* ctx); ///< Cleanup function for context.
    } nil_xit_tagged_value_accessor;

    /**
     * @brief Group entry for core group configuration.
     *
     * Used with nil_xit_set_groups.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_group_entry
    {
        const char* group; ///< Group name.
        const char* path;  ///< Path associated with group.
    } nil_xit_group_entry;

    /**
     * @brief File info used when registering frames.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_file_info
    {
        const char* group; ///< Group name.
        const char* path;  ///< Path relative to group.
    } nil_xit_file_info;

    /**
     * @brief Creates a new nil-xit core instance.
     *
     * @param run_service Service runner callback.
     * @param event_service Event handler callback.
     * @return Handle to the created core.
     */
    nil_xit_core nil_xit_core_create(
        nil_service_runnable run_service,
        nil_service_event event_service
    );

    /**
     * @brief Creates a core from a standalone service.
     *
     * @param service Standalone service handle.
     * @return Handle to the created core.
     */
    nil_xit_core nil_xit_core_create_from_standalone(nil_service_standalone service);

    /**
     * @brief Sets up a web server for the service.
     *
     * @param service Web service handle.
     * @param asset_paths Array of paths to static assets.
     * @param count Number of asset paths.
     *
     * This replaces the single path with a list of paths (to match the C++ vector<string>).
     */
    void nil_xit_setup_server(nil_service_web service, const char** asset_paths, size_t count);

    /**
     * @brief Sets the cache directory for the core.
     *
     * @param core Core handle.
     * @param tmp_path Path to cache directory or NULL to disable caching.
     */
    void nil_xit_set_cache_directory(nil_xit_core core, const char* tmp_path);

    /**
     * @brief Sets the groups for the core.
     *
     * @param core Core handle.
     * @param groups Array of group entries.
     * @param size Number of group entries.
     */
    void nil_xit_set_groups(nil_xit_core core, const nil_xit_group_entry* groups, uint64_t size);

    /**
     * @brief Destroys a core instance and releases resources.
     *
     * @param core Core handle to destroy.
     */
    void nil_xit_core_destroy(nil_xit_core core);

    /**
     * @brief Opaque handle to a tagged frame.
     *
     * Use only with nil_xit_tagged_frame_* API functions.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_tagged_frame
    {
        void* handle;
    } nil_xit_tagged_frame;

    /**
     * @brief Adds a unique frame to the core.
     *
     * @param core Core handle.
     * @param id Frame identifier.
     * @param file_info Optional file info with group and path (pass NULL when unused).
     * @return Handle to the created unique frame.
     */
    nil_xit_unique_frame nil_xit_core_add_unique_frame(
        nil_xit_core core,
        const char* id,
        const nil_xit_file_info* file_info
    );

    /**
     * @brief Adds a tagged frame to the core.
     *
     * @param core Core handle.
     * @param id Frame identifier.
     * @param file_info Optional file info with group and path (pass NULL when unused).
     * @return Handle to the created tagged frame.
     */
    nil_xit_tagged_frame nil_xit_core_add_tagged_frame(
        nil_xit_core core,
        const char* id,
        const nil_xit_file_info* file_info
    );

    /**
     * @brief Callback info for tagged frame events.
     *
     * Used for registering callbacks with context and cleanup. The callback receives a tag
     * string as the first argument.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_tagged_callback_info
    {
        void (*exec)(const char* tag, void* context);
        void* context;
        void (*cleanup)(void*);
    } nil_xit_tagged_callback_info;

    /**
     * @brief Registers a callback for the frame load event.
     *
     * @param frame Unique frame handle.
     * @param callback Callback info struct.
     */
    void nil_xit_unique_frame_on_load(
        nil_xit_unique_frame frame,
        nil_xit_unique_callback_info callback
    );

    /**
     * @brief Registers a callback for the tagged frame load event.
     *
     * @param frame Tagged frame handle.
     * @param callback Callback info struct.
     */
    void nil_xit_tagged_frame_on_load(
        nil_xit_tagged_frame frame,
        nil_xit_tagged_callback_info callback
    );

    /**
     * @brief Callback info for tagged frame subscription events.
     *
     * Used for on_sub event registration. The callback receives a tag string as the first
     * argument.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_tagged_on_sub_info
    {
        void (*exec)(const char* tag, uint64_t count, void* context);
        void* context;
        void (*cleanup)(void*);
    } nil_xit_tagged_on_sub_info;

    /**
     * @brief Registers a callback for the frame subscription event.
     *
     * @param frame Unique frame handle.
     * @param callback Subscription callback info struct.
     */
    void nil_xit_unique_frame_on_sub(
        nil_xit_unique_frame frame,
        nil_xit_unique_on_sub_info callback
    );

    /**
     * @brief Registers a callback for the tagged frame subscription event.
     *
     * @param frame Tagged frame handle.
     * @param callback Subscription callback info struct.
     */
    void nil_xit_tagged_frame_on_sub(
        nil_xit_tagged_frame frame,
        nil_xit_tagged_on_sub_info callback
    );

    /**
     * @brief Opaque handle to a tagged frame value.
     *
     * Provided by the library. Contains encode helpers for posting values.
     */
    // NOLINTNEXTLINE(modernize-use-using)
    typedef struct nil_xit_tagged_frame_value
    {
        void* handle;
    } nil_xit_tagged_frame_value;

    /**
     * @brief Adds a value to a unique frame.
     *
     * @param frame Unique frame handle.
     * @param id Value identifier.
     * @param accessor Value accessor struct.
     * @return Handle to the created frame value.
     */
    nil_xit_unique_frame_value nil_xit_unique_frame_add_value(
        nil_xit_unique_frame frame,
        const char* id,
        nil_xit_unique_value_accessor accessor
    );

    /**
     * @brief Adds a value to a tagged frame.
     *
     * @param frame Tagged frame handle.
     * @param id Value identifier.
     * @param accessor Value accessor struct.
     * @return Handle to the created frame value.
     */
    nil_xit_tagged_frame_value nil_xit_tagged_frame_add_value(
        nil_xit_tagged_frame frame,
        const char* id,
        nil_xit_tagged_value_accessor accessor
    );

    /**
     * @brief Adds an option to a unique frame.
     *
     * @param frame Unique frame handle.
     * @param key Option key.
     * @param value Option value.
     */
    void nil_xit_unique_frame_add_option(
        nil_xit_unique_frame frame,
        const char* key,
        const char* value
    );

    /**
     * @brief Adds an option to a tagged frame.
     *
     * @param frame Tagged frame handle.
     * @param key Option key.
     * @param value Option value.
     */
    void nil_xit_tagged_frame_add_option(
        nil_xit_tagged_frame frame,
        const char* key,
        const char* value
    );

    /**
     * @brief Adds a signal to a unique frame.
     *
     * Signals are events without associated value payloads.
     * Registers a callback to be invoked when the signal is triggered.
     *
     * @param frame Unique frame handle.
     * @param id Signal identifier.
     * @param callback Callback info struct (exec, context, cleanup).
     */
    // NOLINTNEXTLINE(modernize-use-using)
    void nil_xit_unique_frame_add_signal(
        nil_xit_unique_frame frame,
        const char* id,
        nil_xit_unique_callback_info callback
    );

    /**
     * @brief Adds a signal to a tagged frame.
     *
     * Signals are events without associated value payloads.
     * Registers a callback to be invoked when the signal is triggered. The callback receives
     * the tag string as the first argument.
     *
     * @param frame Tagged frame handle.
     * @param id Signal identifier.
     * @param callback Callback info struct (exec, context, cleanup).
     */
    // NOLINTNEXTLINE(modernize-use-using)
    void nil_xit_tagged_frame_add_signal(
        nil_xit_tagged_frame frame,
        const char* id,
        nil_xit_tagged_callback_info callback
    );

    /**
     * @brief Posts a new value for the given unique frame value.
     *
     * @param value Frame value handle.
     * @param new_data Pointer to the new value (must match accessor's ctx type).
     *
     * @note
     * - `new_data` is not owned by the library: it will not be freed, retained, or modified, and is
     *   only used for initial serialization (encode) during this call.
     * - The accessor's encode function is called immediately to serialize `new_data`.
     * - Later, the accessor's decode function will be called with the encoded buffer.
     * - Only data present in the encoded buffer will be stored; any extra fields in `new_data` not
     *   encoded will be lost.
     */
    void nil_xit_unique_value_post(
        nil_xit_unique_frame_value value,
        const void* new_data,
        uint64_t new_data_size
    );

    /**
     * @brief Posts a new value for the given tagged frame value.
     *
     * @param value Frame value handle.
     * @param tag Tag string.
     * @param new_data Pointer to the new value (must match accessor's ctx type).
     *
     * @note
     * - `new_data` is not owned by the library: it will not be freed, retained, or modified,
     *   and is only used for initial serialization (encode) during this call.
     * - The accessor's encode function is called immediately to serialize `new_data`.
     * - Later, the accessor's decode function will be called with the encoded buffer.
     * - Only data present in the encoded buffer will be stored; any extra fields in `new_data`
     *   not encoded will be lost.
     */
    void nil_xit_tagged_value_post(
        nil_xit_tagged_frame_value value,
        const char* tag,
        const void* new_data,
        uint64_t new_data_size
    );
#ifdef __cplusplus
}
#endif
