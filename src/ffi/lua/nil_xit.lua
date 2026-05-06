local ffi = require("ffi")
local nil_service = require("nil_service")

ffi.cdef [[
    typedef struct nil_xit_core               { void* handle; } nil_xit_core;
    typedef struct nil_xit_unique_frame       { void* handle; } nil_xit_unique_frame;
    typedef struct nil_xit_unique_frame_value { void* handle; } nil_xit_unique_frame_value;
    typedef struct nil_xit_tagged_frame       { void* handle; } nil_xit_tagged_frame;
    typedef struct nil_xit_tagged_frame_value { void* handle; } nil_xit_tagged_frame_value;

    typedef struct nil_xit_group_entry
    {
        const char* group;
        const char* path;
    } nil_xit_group_entry;

    typedef struct nil_xit_file_info
    {
        const char* group;
        const char* path;
    } nil_xit_file_info;

    typedef struct nil_xit_unique_callback_info
    {
        void (*exec)(void*);
        void* context;
        void (*cleanup)(void*);
    } nil_xit_unique_callback_info;

    typedef struct nil_xit_unique_on_sub_info
    {
        void (*exec)(uint64_t count, void*);
        void* context;
        void (*cleanup)(void*);
    } nil_xit_unique_on_sub_info;

    typedef struct nil_xit_tagged_callback_info
    {
        void (*exec)(const char* tag, void* context);
        void* context;
        void (*cleanup)(void*);
    } nil_xit_tagged_callback_info;

    typedef struct nil_xit_tagged_on_sub_info
    {
        void (*exec)(const char* tag, uint64_t count, void* context);
        void* context;
        void (*cleanup)(void*);
    } nil_xit_tagged_on_sub_info;

    typedef struct nil_xit_unique_value_accessor
    {
        uint64_t (*encode_size)(const void* ctx);
        void     (*encode)(const void* ctx, void* buffer);
        void     (*decode)(void* ctx, const void* buffer, uint64_t size);
        void*    ctx;
        void     (*cleanup)(void* ctx);
    } nil_xit_unique_value_accessor;

    typedef struct nil_xit_tagged_value_accessor
    {
        uint64_t (*encode_size)(const char* tag, const void* ctx);
        void     (*encode)(const char* tag, const void* ctx, void* buffer);
        void     (*decode)(const char* tag, void* ctx, const void* buffer, uint64_t size);
        void*    ctx;
        void     (*cleanup)(void* ctx);
    } nil_xit_tagged_value_accessor;

    nil_xit_core nil_xit_core_create(nil_service_runnable, nil_service_event);
    nil_xit_core nil_xit_core_create_from_standalone(nil_service_standalone);
    void         nil_xit_setup_server(nil_service_web, const char** asset_paths, size_t count);
    void         nil_xit_set_cache_directory(nil_xit_core, const char* tmp_path);
    void         nil_xit_set_groups(nil_xit_core, const nil_xit_group_entry* groups, uint64_t size);
    void         nil_xit_core_destroy(nil_xit_core);

    nil_xit_unique_frame nil_xit_core_add_unique_frame(nil_xit_core, const char* id, const nil_xit_file_info* file_info);
    nil_xit_tagged_frame nil_xit_core_add_tagged_frame(nil_xit_core, const char* id, const nil_xit_file_info* file_info);

    void nil_xit_unique_frame_on_load(nil_xit_unique_frame, nil_xit_unique_callback_info);
    void nil_xit_unique_frame_on_sub(nil_xit_unique_frame, nil_xit_unique_on_sub_info);
    void nil_xit_tagged_frame_on_load(nil_xit_tagged_frame, nil_xit_tagged_callback_info);
    void nil_xit_tagged_frame_on_sub(nil_xit_tagged_frame, nil_xit_tagged_on_sub_info);

    nil_xit_unique_frame_value nil_xit_unique_frame_add_value(nil_xit_unique_frame, const char* id, nil_xit_unique_value_accessor);
    nil_xit_tagged_frame_value nil_xit_tagged_frame_add_value(nil_xit_tagged_frame, const char* id, nil_xit_tagged_value_accessor);

    void nil_xit_unique_frame_add_option(nil_xit_unique_frame, const char* key, const char* value);
    void nil_xit_tagged_frame_add_option(nil_xit_tagged_frame, const char* key, const char* value);

    void nil_xit_unique_frame_add_signal(nil_xit_unique_frame, const char* id, nil_xit_unique_callback_info);
    void nil_xit_tagged_frame_add_signal(nil_xit_tagged_frame, const char* id, nil_xit_tagged_callback_info);

    void nil_xit_unique_value_post(nil_xit_unique_frame_value, const void* data, uint64_t size);
    void nil_xit_tagged_value_post(nil_xit_tagged_frame_value, const char* tag, const void* data, uint64_t size);
]]

local function to_ref_id(id)
    return tonumber(ffi.cast("uintptr_t", id))
end

local function current_file_dir()
    local source = debug.getinfo(1, "S").source
    if source:sub(1, 1) == "@" then
        source = source:sub(2)
    end

    local dir = source:match("(.*/)")
    if dir == nil then
        return "./"
    end

    return dir
end

---@class nil_xit.UniqueValue
---@field post fun(self: nil_xit.UniqueValue, data: unknown)

---@class nil_xit.TaggedValue
---@field post fun(self: nil_xit.TaggedValue, tag: string, data: unknown)

---@class nil_xit.UniqueValueAccessor
---@field encode fun(): unknown
---@field decode fun(data: unknown)

---@class nil_xit.TaggedValueAccessor
---@field encode fun(tag: string): unknown
---@field decode fun(tag: string, data: unknown)

---@class nil_xit.UniqueFrame
---@field on_load    fun(self: nil_xit.UniqueFrame, fn: fun())
---@field on_sub     fun(self: nil_xit.UniqueFrame, fn: fun(count: number))
---@field add_value  fun(self: nil_xit.UniqueFrame, id: string, accessor: nil_xit.UniqueValueAccessor): nil_xit.UniqueValue
---@field add_option fun(self: nil_xit.UniqueFrame, key: string, value: string)
---@field add_signal fun(self: nil_xit.UniqueFrame, id: string, fn: fun())

---@class nil_xit.TaggedFrame
---@field on_load    fun(self: nil_xit.TaggedFrame, fn: fun(tag: string))
---@field on_sub     fun(self: nil_xit.TaggedFrame, fn: fun(tag: string, count: number))
---@field add_value  fun(self: nil_xit.TaggedFrame, id: string, accessor: nil_xit.TaggedValueAccessor): nil_xit.TaggedValue
---@field add_option fun(self: nil_xit.TaggedFrame, key: string, value: string)
---@field add_signal fun(self: nil_xit.TaggedFrame, id: string, fn: fun(tag: string))

---@class nil_xit.Core
---@field set_cache_directory fun(self: nil_xit.Core, path: string)
---@field set_groups          fun(self: nil_xit.Core, groups: table<string, string>)
---@field add_unique_frame    fun(self: nil_xit.Core, id: string, info: { group: string, path: string }|nil): nil_xit.UniqueFrame
---@field add_tagged_frame    fun(self: nil_xit.Core, id: string, info: { group: string, path: string }|nil): nil_xit.TaggedFrame
---@field destroy             fun(self: nil_xit.Core)

---@class nil_xit.Module
---@field create_core               fun(runnable: unknown, event: unknown): nil_xit.Core
---@field create_core_from_standalone fun(standalone: unknown): nil_xit.Core
---@field setup_server              fun(web: unknown, paths: string[])


local function create_unique_value(refs, fns, lib, value)
    return {
        _value = value,
        post = function(self, data)
            lib.nil_xit_unique_value_post(self._value, data, ffi.sizeof(data))
        end,
    }
end

local function create_tagged_value(refs, fns, lib, value)
    return {
        _value = value,
        post = function(self, tag, data)
            lib.nil_xit_tagged_value_post(self._value, tag, data, ffi.sizeof(data))
        end,
    }
end

local function create_unique_frame(refs, fns, lib, frame)
    return {
        _frame = frame,
        on_load = function(self, fn)
            local info = ffi.new("nil_xit_unique_callback_info")
            info.exec = fns.unique_exec
            info.cleanup = fns.cleanup
            info.context = fns.store_callback(fn)
            lib.nil_xit_unique_frame_on_load(self._frame, info)
        end,
        on_sub = function(self, fn)
            local info = ffi.new("nil_xit_unique_on_sub_info")
            info.exec = fns.unique_on_sub_exec
            info.cleanup = fns.cleanup
            info.context = fns.store_callback(fn)
            lib.nil_xit_unique_frame_on_sub(self._frame, info)
        end,
        add_value = function(self, id, accessor)
            local acc = ffi.new("nil_xit_unique_value_accessor")
            acc.encode_size = fns.unique_encode_size_exec
            acc.encode      = fns.unique_encode_exec
            acc.decode      = fns.unique_decode_exec
            acc.cleanup     = fns.cleanup
            acc.ctx         = fns.store_callback({ encode_fn = accessor.encode, decode_fn = accessor.decode, _cached = nil })
            local value = lib.nil_xit_unique_frame_add_value(self._frame, id, acc)
            return create_unique_value(refs, fns, lib, value)
        end,
        add_option = function(self, key, value)
            lib.nil_xit_unique_frame_add_option(self._frame, key, value)
        end,
        add_signal = function(self, id, fn)
            local info = ffi.new("nil_xit_unique_callback_info")
            info.exec = fns.unique_exec
            info.cleanup = fns.cleanup
            info.context = fns.store_callback(fn)
            lib.nil_xit_unique_frame_add_signal(self._frame, id, info)
        end,
    }
end

local function create_tagged_frame(refs, fns, lib, frame)
    return {
        _frame = frame,
        on_load = function(self, fn)
            local info = ffi.new("nil_xit_tagged_callback_info")
            info.exec = fns.tagged_exec
            info.cleanup = fns.cleanup
            info.context = fns.store_callback(fn)
            lib.nil_xit_tagged_frame_on_load(self._frame, info)
        end,
        on_sub = function(self, fn)
            local info = ffi.new("nil_xit_tagged_on_sub_info")
            info.exec = fns.tagged_on_sub_exec
            info.cleanup = fns.cleanup
            info.context = fns.store_callback(fn)
            lib.nil_xit_tagged_frame_on_sub(self._frame, info)
        end,
        add_value = function(self, id, accessor)
            local acc = ffi.new("nil_xit_tagged_value_accessor")
            acc.encode_size = fns.tagged_encode_size_exec
            acc.encode      = fns.tagged_encode_exec
            acc.decode      = fns.tagged_decode_exec
            acc.cleanup     = fns.cleanup
            acc.ctx         = fns.store_callback({ encode_fn = accessor.encode, decode_fn = accessor.decode, _cached = nil })
            local value = lib.nil_xit_tagged_frame_add_value(self._frame, id, acc)
            return create_tagged_value(refs, fns, lib, value)
        end,
        add_option = function(self, key, value)
            lib.nil_xit_tagged_frame_add_option(self._frame, key, value)
        end,
        add_signal = function(self, id, fn)
            local info = ffi.new("nil_xit_tagged_callback_info")
            info.exec = fns.tagged_exec
            info.cleanup = fns.cleanup
            info.context = fns.store_callback(fn)
            lib.nil_xit_tagged_frame_add_signal(self._frame, id, info)
        end,
    }
end

local function create_core(refs, fns, lib, core)
    return {
        _core = core,
        set_cache_directory = function(self, path)
            lib.nil_xit_set_cache_directory(self._core, path)
        end,
        set_groups = function(self, groups)
            local count = 0
            for _ in pairs(groups) do count = count + 1 end
            local entries = ffi.new("nil_xit_group_entry[?]", count)
            local i = 0
            for group, path in pairs(groups) do
                entries[i].group = group
                entries[i].path  = path
                i = i + 1
            end
            lib.nil_xit_set_groups(self._core, entries, count)
        end,
        add_unique_frame = function(self, id, info)
            local file_info = nil
            if info ~= nil then
                if type(info) ~= "table" or info.group == nil or info.path == nil then
                    error("info must be a table with group and path")
                end
                file_info = ffi.new("nil_xit_file_info")
                file_info.group = info.group
                file_info.path = info.path
            end
            local frame = lib.nil_xit_core_add_unique_frame(self._core, id, file_info)
            return create_unique_frame(refs, fns, lib, frame)
        end,
        add_tagged_frame = function(self, id, info)
            local file_info = nil
            if info ~= nil then
                if type(info) ~= "table" or info.group == nil or info.path == nil then
                    error("info must be a table with group and path")
                end
                file_info = ffi.new("nil_xit_file_info")
                file_info.group = info.group
                file_info.path = info.path
            end
            local frame = lib.nil_xit_core_add_tagged_frame(self._core, id, file_info)
            return create_tagged_frame(refs, fns, lib, frame)
        end,
        destroy = function(self)
            lib.nil_xit_core_destroy(self._core)
        end,
    }
end

local function create_lib_fns(refs, lib)
    local unique_exec = ffi.cast(
        "void (*)(void*)",
        function(ctx_id)
            local cb = refs[to_ref_id(ctx_id)]
            if cb then cb() end
        end
    )

    local unique_on_sub_exec = ffi.cast(
        "void (*)(uint64_t, void*)",
        function(count, ctx_id)
            local cb = refs[to_ref_id(ctx_id)]
            if cb then cb(tonumber(count)) end
        end
    )

    local tagged_exec = ffi.cast(
        "void (*)(const char*, void*)",
        function(tag, ctx_id)
            local cb = refs[to_ref_id(ctx_id)]
            if cb then cb(ffi.string(tag)) end
        end
    )

    local tagged_on_sub_exec = ffi.cast(
        "void (*)(const char*, uint64_t, void*)",
        function(tag, count, ctx_id)
            local cb = refs[to_ref_id(ctx_id)]
            if cb then cb(ffi.string(tag), tonumber(count)) end
        end
    )

    local unique_encode_size_exec = ffi.cast(
        "uint64_t (*)(const void*)",
        function(ctx_id)
            local entry = refs[to_ref_id(ctx_id)]
            if entry then
                entry._cached = entry.encode_fn()
                return ffi.sizeof(entry._cached)
            end
            return 0
        end
    )

    local unique_encode_exec = ffi.cast(
        "void (*)(const void*, void*)",
        function(ctx_id, buffer)
            local entry = refs[to_ref_id(ctx_id)]
            if entry and entry._cached then
                ffi.copy(buffer, entry._cached, ffi.sizeof(entry._cached))
                entry._cached = nil
            end
        end
    )

    local unique_decode_exec = ffi.cast(
        "void (*)(void*, const void*, uint64_t)",
        function(ctx_id, buffer, size)
            local entry = refs[to_ref_id(ctx_id)]
            if entry then
                size = tonumber(size)
                local arr = ffi.new("uint8_t[?]", size)
                ffi.copy(arr, buffer, size)
                entry.decode_fn(arr)
            end
        end
    )

    local tagged_encode_size_exec = ffi.cast(
        "uint64_t (*)(const char*, const void*)",
        function(tag, ctx_id)
            local entry = refs[to_ref_id(ctx_id)]
            if entry then
                entry._cached = entry.encode_fn(ffi.string(tag))
                return ffi.sizeof(entry._cached)
            end
            return 0
        end
    )

    local tagged_encode_exec = ffi.cast(
        "void (*)(const char*, const void*, void*)",
        function(tag, ctx_id, buffer)
            local entry = refs[to_ref_id(ctx_id)]
            if entry and entry._cached then
                ffi.copy(buffer, entry._cached, ffi.sizeof(entry._cached))
                entry._cached = nil
            end
        end
    )

    local tagged_decode_exec = ffi.cast(
        "void (*)(const char*, void*, const void*, uint64_t)",
        function(tag, ctx_id, buffer, size)
            local entry = refs[to_ref_id(ctx_id)]
            if entry then
                size = tonumber(size)
                local arr = ffi.new("uint8_t[?]", size)
                ffi.copy(arr, buffer, size)
                entry.decode_fn(ffi.string(tag), arr)
            end
        end
    )

    local cleanup = ffi.cast(
        "void (*)(void*)",
        function(ctx_id)
            if ctx_id ~= nil then
                refs[to_ref_id(ctx_id)] = nil
                ffi.C.free(ctx_id)
            end
        end
    )

    local store_callback = function(data)
        local id = ffi.C.malloc(1)
        refs[to_ref_id(id)] = data
        return id
    end

    return {
        unique_exec              = unique_exec,
        unique_on_sub_exec       = unique_on_sub_exec,
        tagged_exec              = tagged_exec,
        tagged_on_sub_exec       = tagged_on_sub_exec,
        unique_encode_size_exec  = unique_encode_size_exec,
        unique_encode_exec       = unique_encode_exec,
        unique_decode_exec       = unique_decode_exec,
        tagged_encode_size_exec  = tagged_encode_size_exec,
        tagged_encode_exec       = tagged_encode_exec,
        tagged_decode_exec       = tagged_decode_exec,
        cleanup                  = cleanup,
        store_callback           = store_callback,
    }
end

local function load_library()
    local lib_path = current_file_dir() .. "libnil-xit-c-api.so"
    return ffi.load(lib_path)
end

local function create_xit_lib()
    local lib = load_library()
    local refs = {}
    local fns  = create_lib_fns(refs, lib)

    return {
        ---@return nil_xit.Core
        create_core = function(runnable, event)
            local core = lib.nil_xit_core_create(runnable._runnable, event._event)
            return create_core(refs, fns, lib, core)
        end,

        ---@return nil_xit.Core
        create_core_from_standalone = function(standalone)
            local core = lib.nil_xit_core_create_from_standalone(standalone._standalone)
            return create_core(refs, fns, lib, core)
        end,

        setup_server = function(http, paths)
            -- Extract handle from nil_service wrapper object if needed
            local count = #paths
            local arr = ffi.new("const char*[?]", count)
            for i, p in ipairs(paths) do
                arr[i - 1] = p
            end
            lib.nil_xit_setup_server(http._web, arr, count)
        end
    }
end

return create_xit_lib()