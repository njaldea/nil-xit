package.path      = package.path .. ";../src/ffi/lua/?.lua"

local ffi         = require("ffi")
local nil_service = require("nil_service")
local nil_xit     = require("nil_xit")
local bit         = require("bit")

-- FFI declarations for non-blocking stdin check
ffi.cdef [[
    typedef struct {
        int fds_bits[32];
    } fd_set;

    int select(int nfds, fd_set* readfds, fd_set* writefds,
               fd_set* exceptfds, struct timeval* timeout);

    struct timeval {
        long tv_sec;
        long tv_usec;
    };

    int usleep(unsigned int useconds);
]]

local function has_input_available()
    local tv = ffi.new("struct timeval")
    tv.tv_sec = 0
    tv.tv_usec = 1000 -- 1ms timeout

    local readfds = ffi.new("fd_set")
    ffi.fill(readfds, ffi.sizeof("fd_set"), 0)

    -- Set bit for stdin (fd 0)
    -- fd_set uses bit manipulation: FD_SET(fd, &set) -> set.fds_bits[fd/32] |= (1 << (fd%32))
    local fd = 0
    readfds.fds_bits[fd / 32] = bit.bor(readfds.fds_bits[fd / 32], bit.lshift(1, fd % 32))

    local result = ffi.C.select(1, readfds, nil, nil, tv)
    return result > 0
end

-- Helper to get the script directory
local function get_script_dir()
    local source = debug.getinfo(1, "S").source
    if source:sub(1, 1) == "@" then
        source = source:sub(2)
    end
    local dir = source:match("(.*/)")
    return dir or "./"
end

local source_dir = get_script_dir()

-- Create HTTP server on 127.0.0.1:1101
local http = nil_service.create_http_server("127.0.0.1", 1101, 100 * 1024 * 1024)

-- Setup server with asset paths
nil_xit.setup_server(http, {
    "assets",
    "assets/xit/assets"
})

-- Create websocket event service
local ws = http:use_ws("/ws")

-- Create core from web service and websocket event
local core = nil_xit.create_core(http, ws)

-- -- -- Set cache directory
-- -- local tmp_dir = os.tmpname() .. "_sandbox"
-- -- -- Remove if exists (best effort)
-- -- pcall(function()
-- --     for file in lfs.dir(tmp_dir) do
-- --         if file ~= "." and file ~= ".." then
-- --             os.remove(tmp_dir .. "/" .. file)
-- --         end
-- --     end
-- --     lfs.rmdir(tmp_dir)
-- -- end)
-- -- lfs.mkdir(tmp_dir)
-- -- core:set_cache_directory(tmp_dir)

-- Set groups
core:set_groups({
    base       = source_dir,
    components = source_dir .. "gui/components"
})

-- Initialize all frames
core:add_unique_frame("index", "$base/gui/Demo.svelte")
local base_frame = core:add_unique_frame("base", "$base/gui/Base.svelte")
core:add_unique_frame("group", "$base/gui/Group.svelte")
core:add_unique_frame("json_editor", "$base/gui/JsonEditor.svelte")
local tagged_frame = core:add_tagged_frame("tagged", "$base/gui/Tagged.svelte")


local str_value_g = ffi.new("uint8_t[?]", #"hello world")
ffi.copy(str_value_g, "hello world", #"hello world")

-- Create a string value for input in the base frame
local str_value = base_frame:add_value("value_0_1", {
    encode = function()
        return str_value_g
    end,
    decode = function(buffer)
        str_value_g = ffi.string(buffer, ffi.sizeof(buffer))
        io.write("value changed: " .. str_value_g .. "\n")
    end
})

-- Add signal-1: forces value update
base_frame:add_signal("signal-1", function()
    io.write("signal-1 is notified, forcing value_0_1 value\n")
    local new_str = "new stuff here"
    local buffer = ffi.new("uint8_t[?]", #new_str)
    ffi.copy(buffer, new_str, #new_str)
    str_value:post(buffer)
end)

-- Add signal-2: receives JSON
base_frame:add_signal("signal-2", function()
    io.write("signal-2 is notified\n")
end)

-- Add signal-3: receives boolean
base_frame:add_signal("signal-3", function()
    io.write("signal-3 is notified\n")
end)

-- Register on_ready callback
http:on_ready(function(id)
    print("http://" .. id:to_string())
end)

while true do
    http:poll()

    -- Non-blocking check for input availability
    if has_input_available() then
        local line = io.read("*l")
        if line then
            if line == "quit" or line == "exit" then
                break
            end
            -- Convert string to buffer and post
            local buffer = ffi.new("uint8_t[?]", #line)
            ffi.copy(buffer, line, #line)
            str_value:post(buffer)
            io.write("input here: ")
            io.flush()
        end
    else
        -- Sleep for 1ms to avoid busy-waiting
        ffi.C.usleep(1000)
    end
end
