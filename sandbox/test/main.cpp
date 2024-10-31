#include <nil/gate/Core.hpp>
#include <nil/gate/edges/Mutable.hpp>
#include <nil/xit.hpp>

#include <nil/gate.hpp>
#include <nil/gate/bias/nil.hpp>
#include <nil/gate/runners/NonBlocking.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace transparent
{
    struct Hash
    {
        using is_transparent = void;

        std::size_t operator()(std::string_view s) const
        {
            return std::hash<std::string_view>()(s);
        }
    };

    struct Equal
    {
        using is_transparent = void;

        bool operator()(std::string_view l, std::string_view r) const
        {
            return l == r;
        }
    };
}

struct RerunTag
{
};

bool operator==(const RerunTag& /* l */, const RerunTag& /* r */)
{
    return false;
}

namespace nil::xit
{
    template <>
    struct buffer_type<nlohmann::json>
    {
        static nlohmann::json deserialize(const void* data, std::uint64_t size)
        {
            return nlohmann::json::parse(std::string_view(static_cast<const char*>(data), size));
        }

        static std::vector<std::uint8_t> serialize(const nlohmann::json& value)
        {
            auto s = value.dump();
            return {s.begin(), s.end()};
        }
    };
}

struct App;

struct IItem
{
    IItem() = default;
    virtual ~IItem() = default;
    IItem(IItem&&) = delete;
    IItem(const IItem&) = delete;
    IItem& operator=(IItem&&) = delete;
    IItem& operator=(const IItem&) = delete;

    virtual const nlohmann::json& get_input_data() const = 0;
    virtual void set_input_data(nlohmann::json data) = 0;
    virtual void send_output_data(
        std::string_view tag,
        nil::xit::tagged::Value<nlohmann::json>* value,
        const nlohmann::json& data
    ) = 0;
    virtual void rerun() = 0;
};

template <typename T>
struct Identity
{
    static constexpr bool id = false;
};

struct App
{
    void add_item(nil::xit::Core& xit, std::string_view tag)
    {
        struct InputFrame
        {
            static void install_frame(nil::xit::Core& core, App& app)
            {
                auto& editor_frame
                    = add_tagged_frame(core, "editor_frame", "gui/EditorFrame.svelte");
                add_value(
                    editor_frame,
                    "scene",
                    [&](std::string_view tag) { return app.get_input_data(tag); },
                    [&](std::string_view tag, nlohmann::json new_data)
                    { app.set_input_data(tag, std::move(new_data)); }
                );
            }

            void install_gate(nil::gate::Core& core)
            {
                data = nlohmann::json::array({nlohmann::json::object(
                    {{"x", nlohmann::json::array({"giraffes", "orangutans", "monkeys"})},
                     {"y", nlohmann::json::array({20, 14, 23})},
                     {"type", "bar"}}
                )});
                input = core.edge(data);
            }

            nlohmann::json data;
            nil::gate::edges::Mutable<nlohmann::json>* input = nullptr;
        };

        struct OutputFrame
        {
            static auto* install_frame(nil::xit::Core& core, App& app)
            {
                auto& view_frame = add_tagged_frame(
                    core,
                    "view_frame",
                    "gui/ViewFrame.svelte",
                    [&](auto tag) { app.rerun(tag); }
                );
                return &add_value(view_frame, "scene", [](auto) { return nlohmann::json(); });
            }

            void install_gate(nil::gate::Core& core)
            {
                rerun = core.edge(RerunTag());
            }

            nil::gate::edges::Mutable<RerunTag>* rerun;
        };

        // TODO: generalize this item
        struct Item: IItem
        {
            Item(nil::xit::Core& xit, App& app, std::string_view tag)
            {
                std::apply(
                    [&](auto&... i)
                    {
                        (app.install_input_frame<std::decay_t<decltype(i)>>( //
                             xit,
                             &Identity<std::decay_t<decltype(i)>>::id
                         ),
                         ...);
                        (i.install_gate(app.gate), ...);
                    },
                    inputs
                );
                std::apply(
                    [&](auto&... i)
                    {
                        (app.install_output_frame<std::decay_t<decltype(i)>>( //
                             xit,
                             &Identity<std::decay_t<decltype(i)>>::id
                         ),
                         ...);
                        (i.install_gate(app.gate), ...);
                    },
                    outputs
                );

                auto [output] = app.gate.node(
                    [tag](const nlohmann::json& input_data)
                    {
                        std::cout << "run (test) " << tag << std::endl;
                        return input_data;
                    },
                    {std::get<0>(inputs).input}
                );
                app.gate.node(
                    [&, tag](RerunTag, const nlohmann::json& output_data)
                    {
                        std::cout << "out (test) " << tag << std::endl;
                        app.send_output_data(tag, &Identity<OutputFrame>::id, output_data);
                    },
                    {std::get<0>(outputs).rerun, output}
                );
            }

            void set_input_data(nlohmann::json new_data) override
            {
                std::get<0>(inputs).data = new_data;
                std::get<0>(inputs).input->set_value(std::move(new_data));
            }

            const nlohmann::json& get_input_data() const override
            {
                return std::get<0>(inputs).data;
            }

            void send_output_data(
                std::string_view tag,
                nil::xit::tagged::Value<nlohmann::json>* value,
                const nlohmann::json& data
            ) override
            {
                post(tag, *value, data);
            }

            void rerun() override
            {
                std::apply([](const auto&... o) { (o.rerun->set_value({}), ...); }, outputs);
            }

            std::tuple<InputFrame> inputs;
            std::tuple<OutputFrame> outputs;
        };

        items.emplace(tag, std::make_unique<Item>(xit, *this, tag));
    }

    nlohmann::json get_input_data(std::string_view tag)
    {
        if (const auto& it = items.find(tag); it != items.end())
        {
            return it->second->get_input_data();
        }
        return {};
    }

    void set_input_data(std::string_view tag, nlohmann::json new_value)
    {
        if (const auto& it = items.find(tag); it != items.end())
        {
            it->second->set_input_data(std::move(new_value));
            gate.commit();
        }
    }

    void send_output_data(std::string_view tag, const void* type, const nlohmann::json& value)
    {
        if (const auto& it = items.find(tag); it != items.end())
        {
            return it->second->send_output_data(tag, installed_outputs.at(type), value);
        }
    }

    void rerun(std::string_view tag)
    {
        if (const auto& it = items.find(tag); it != items.end())
        {
            it->second->rerun();
            gate.commit();
        }
    }

    void rerun_all()
    {
        for (const auto& item : items)
        {
            item.second->rerun();
        }
        gate.commit();
    }

    template <typename T>
    void install_input_frame(nil::xit::Core& xit, const void* id)
    {
        if (!installed_inputs.contains(id))
        {
            T::install_frame(xit, *this);
            installed_inputs.emplace(id);
        }
    }

    template <typename T>
    void install_output_frame(nil::xit::Core& xit, const void* id)
    {
        if (!installed_outputs.contains(id))
        {
            installed_outputs.emplace(id, T::install_frame(xit, *this));
        }
    }

    nil::gate::Core gate;
    std::unordered_set<const void*> installed_inputs;
    std::unordered_map<const void*, nil::xit::tagged::Value<nlohmann::json>*> installed_outputs;
    std::unordered_multimap<
        std::string,
        std::unique_ptr<IItem>,
        transparent::Hash,
        transparent::Equal>
        items;
};

int main()
{
    const auto source_path = std::filesystem::path(__FILE__).parent_path();
    const auto http_server = nil::xit::make_server({
        .source_path = source_path.parent_path(),
        .port = 1101,
        .buffer_size = 1024ul * 1024ul * 100ul //
    });
    const auto core = nil::xit::make_core(http_server);
    set_relative_directory(core, source_path);

    const auto tmp_dir = std::filesystem::temp_directory_path() / "nil-xit-gtest";
    set_cache_directory(core, tmp_dir);

    auto& main_frame = add_unique_frame(core, "demo", "gui/Main.svelte");
    add_value(main_frame, "scenes", nlohmann::json::parse(R"({ "scenes": ["", "a", "b"] })"));
    add_value(main_frame, "selected", 0L);

    App app;
    app.gate.set_runner<nil::gate::runners::NonBlocking>();
    app.add_item(core, "a");
    app.add_item(core, "b");

    on_ready(http_server, [&]() { app.gate.commit(); });
    start(http_server);
    return 0;
}
