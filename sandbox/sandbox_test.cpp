#include <nil/xit.hpp>

#include <iostream>
#include <unordered_map>

namespace transparent
{
    struct Hash
    {
        using is_transparent = void;

        std::size_t operator()(std::string_view txt) const
        {
            return std::hash<std::string_view>()(txt);
        }

        std::size_t operator()(const std::string& txt) const
        {
            return this->operator()(std::string_view(txt));
        }
    };

    struct Equal
    {
        using is_transparent = void;

        bool operator()(const std::string& lhs, const std::string& rhs) const
        {
            return lhs == rhs;
        }

        bool operator()(std::string_view lhs, const std::string& rhs) const
        {
            return lhs == rhs;
        }
    };
}

struct JSON
{
    std::string data = "{}";
};

using Data = std::unordered_map<std::string, JSON, transparent::Hash, transparent::Equal>;

namespace nil::xit
{
    template <>
    struct buffer_type<JSON>
    {
        static JSON deserialize(const void* data, std::uint64_t size)
        {
            return {.data = std::string(static_cast<const char*>(data), size)};
        }

        static std::vector<std::uint8_t> serialize(const JSON& value)
        {
            return {value.data.begin(), value.data.end()};
        }
    };
}

int main()
{
    const auto source_path = std::filesystem::path(__FILE__).parent_path();
    const auto http_server = nil::xit::make_server({
        .source_path = source_path,
        .port = 1101,
        .buffer_size = 1024ul * 1024ul * 100ul //
    });
    const auto core = nil::xit::make_core(http_server);
    set_relative_directory(core, source_path);

    const auto tmp_dir = std::filesystem::temp_directory_path() / "nil-xit-gtest";
    std::filesystem::remove_all(tmp_dir);
    set_cache_directory(core, tmp_dir);

    auto& main_frame = add_unique_frame(core, "demo", "test/Main.svelte");
    add_value(main_frame, "scenes", JSON{R"({ "scenes": ["", "a", "b"] })"});
    auto& selected_view = add_value(main_frame, "selected", 0L);

    struct App
    {
        Data data;

        JSON get_value(std::string_view tag) const
        {
            if (const auto it = data.find(tag); it != data.end())
            {
                return it->second;
            }
            return {};
        }

        void set_value(std::string_view tag, JSON value)
        {
            if (const auto it = data.find(tag); it != data.end())
            {
                it->second = std::move(value);
            }
            else
            {
                data.emplace(tag, std::move(value));
            }
        }
    };

    auto app = App{.data=Data{
        {"a", JSON(R"({ "hello": true })")},
        {"b", JSON(R"({ "world": true })")} //
    }};

    auto& view_frame = add_tagged_frame(core, "view_frame", "test/ViewFrame.svelte");
    auto& editor_frame = add_tagged_frame(core, "editor_frame", "test/EditorFrame.svelte");
    auto& value = add_value(
        view_frame,
        "scene",
        [&](auto tag) { return app.get_value(tag); },
        [](auto tag, const JSON& v)
        { std::cout << "tag: " << tag << " - " << v.data << std::endl; } //
    );
    add_value(
        editor_frame,
        "scene",
        [&](auto tag) { return app.get_value(tag); },
        [&](std::string_view tag, JSON new_data)
        {
            post(tag, value, new_data);
            app.set_value(tag, std::move(new_data));
        } //
    );

    {
        auto& frame = add_unique_frame(core, "cli", "test/CLI.svelte");
        add_signal(
            frame,
            "message",
            [&](std::string_view message)
            {
                if (message == "unload")
                {
                    post(selected_view, 0);
                }
                else if (message == "load a")
                {
                    post(selected_view, 1);
                }
                else if (message == "load b")
                {
                    post(selected_view, 2);
                }
            }
        );
    }

    start(http_server);
    return 0;
}
