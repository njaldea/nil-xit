#include <nil/xit.hpp>

#include <nil/gate.hpp>
#include <nil/gate/bias/nil.hpp>
#include <nil/gate/runners/NonBlocking.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

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
    bool operator==(const RerunTag& /* o */) const
    {
        return false;
    }
};

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

template <typename T>
struct Identity
{
    static constexpr bool id = false;
};

template <typename T>
    requires(!std::is_same_v<T, std::decay_t<T>>)
struct Identity<T>: Identity<std::decay_t<T>>
{
};

struct IItem
{
    IItem() = default;
    virtual ~IItem() = default;
    IItem(IItem&&) = delete;
    IItem(const IItem&) = delete;
    IItem& operator=(IItem&&) = delete;
    IItem& operator=(const IItem&) = delete;

    template <typename T>
    T* get_input()
    {
        return static_cast<T*>(get_input(&Identity<T>::id));
    }

    template <typename T>
    T* get_output()
    {
        return static_cast<T*>(get_output(&Identity<T>::id));
    }

protected:
    virtual void* get_input(const void*) = 0;
    virtual void* get_output(const void*) = 0;
};

struct App final
{
    explicit App(nil::service::S service)
        : xit(nil::xit::make_core(service))
    {
    }

    ~App() noexcept = default;
    App(App&&) = delete;
    App(const App&) = delete;
    App& operator=(App&&) = delete;
    App& operator=(const App&) = delete;

    template <typename Item, typename Callable>
    void add_item(Callable callable, std::string_view tag)
    {
        items.emplace(tag, std::make_unique<Item>(std::move(callable), *this, tag));
    }

    template <typename T>
    T* get_input(std::string_view tag) const
    {
        if (const auto& it = items.find(tag); it != items.end())
        {
            return it->second->get_input<T>();
        }
        return nullptr;
    }

    template <typename T>
    T* get_output(std::string_view tag) const
    {
        if (const auto& it = items.find(tag); it != items.end())
        {
            return it->second->get_output<T>();
        }
        return nullptr;
    }

    template <typename T>
    nil::xit::tagged::Value<typename T::type>* get_installed_output() const
    {
        const auto* id = &Identity<T>::id;
        if (const auto& it = installed_outputs.find(id); it != installed_outputs.end())
        {
            return static_cast<nil::xit::tagged::Value<typename T::type>*>(it->second);
        }
        return nullptr;
    }

    nil::xit::C xit;
    nil::gate::Core gate;
    std::unordered_set<const void*> installed_inputs;
    std::unordered_map<const void*, void*> installed_outputs;
    std::unordered_map<std::string, std::unique_ptr<IItem>, transparent::Hash, transparent::Equal>
        items;
};

template <typename Parent, typename T>
struct Input
{
    using type = T;

    void install(App& app)
    {
        data = Parent::initialize();
        input = app.gate.edge(data);
    }

    static type get_data(App& app, std::string_view tag)
    {
        if (auto* i = app.get_input<Parent>(tag); i != nullptr)
        {
            return i->data;
        }
        return type();
    }

    static void set_data(App& app, std::string_view tag, type new_data)
    {
        if (auto* i = app.get_input<Parent>(tag); i != nullptr)
        {
            i->data = new_data;
            i->input->set_value(std::move(new_data));
            app.gate.commit();
        }
    }

    type data = {};
    nil::gate::edges::Mutable<type>* input = nullptr;
};

struct InputFrame: Input<InputFrame, nlohmann::json>
{
    static type initialize()
    {
        return nlohmann::json::array({nlohmann::json::object(
            {{"x", nlohmann::json::array({"giraffes", "orangutans", "monkeys"})},
             {"y", nlohmann::json::array({20, 14, 23})},
             {"type", "bar"}}
        )});
    }

    static void install_frame(App& app)
    {
        auto& frame = add_tagged_frame(app.xit, "editor_frame", "gui/EditorFrame.svelte");
        add_value(
            frame,
            "scene",
            [&](std::string_view tag) { return get_data(app, tag); },
            [&](std::string_view tag, type new_data) { set_data(app, tag, std::move(new_data)); }
        );
    }
};

struct SliderInputFrame: Input<SliderInputFrame, std::int64_t>
{
    static type initialize()
    {
        return 0;
    }

    static void install_frame(App& app)
    {
        auto& frame = add_tagged_frame(app.xit, "slider_frame", "gui/Slider.svelte");
        add_value(
            frame,
            "value",
            [&](std::string_view tag) { return get_data(app, tag); },
            [&](std::string_view tag, type new_data) { set_data(app, tag, new_data); }
        );
    }
};

template <typename Parent, typename T>
struct Output
{
    using type = T;

    void install(
        App& app,
        std::string_view tag,
        nil::xit::tagged::Value<T>* value,
        nil::gate::edges::ReadOnly<type>* gate_output
    )
    {
        rerun = app.gate.edge(RerunTag());
        app.gate.node(
            [value, tag](RerunTag, const type& output_data)
            {
                std::cout << "out (test) " << tag << std::endl;
                post(tag, *value, output_data);
            },
            {rerun, gate_output}
        );
    }

    static void frame_loaded(App& app, std::string_view tag)
    {
        if (auto* o = app.get_output<Parent>(tag); o != nullptr)
        {
            o->rerun->set_value({});
            app.gate.commit();
        }
    }

    nil::xit::tagged::Value<type>* output;
    nil::gate::edges::Mutable<RerunTag>* rerun;
};

struct OutputFrame: Output<OutputFrame, nlohmann::json>
{
    static auto* install_frame(App& app)
    {
        auto& view_frame = add_tagged_frame(
            app.xit,
            "view_frame",
            "gui/ViewFrame.svelte",
            [&](std::string_view tag) { frame_loaded(app, tag); }
        );
        return &add_value(view_frame, "scene", [](auto) { return type(); });
    }
};

template <typename Inputs, typename Outputs>
struct Item;

template <typename P, typename A, typename B, std::size_t... I>
void for_each(P predicate, A&& a, B&& b, std::index_sequence<I...> /* indices */)
{
    (([&]() { predicate(get<I>(a), get<I>(b)); }()), ...);
}

struct Callable
{
    std::string tag;

    nlohmann::json operator()(const nlohmann::json& input_data, std::int64_t value) const
    {
        std::cout << "run (test) " << tag << std::endl;
        auto result = input_data;
        result[0]["y"][0] = input_data[0]["y"][0].get<std::int64_t>() * value;
        return result;
    }
};

template <typename... Inputs, typename... Outputs>
struct Item<std::tuple<Inputs...>, std::tuple<Outputs...>>: IItem
{
    template <typename Callable>
    Item(Callable callable, App& app, std::string_view tag)
    {
        const auto gate_inputs = std::apply(
            [&](auto&... i)
            {
                return std::make_tuple(
                    [&]()
                    {
                        using input_t = std::decay_t<decltype(i)>;
                        constexpr const void* id = &Identity<input_t>::id;
                        if (!app.installed_inputs.contains(id))
                        {
                            input_t::install_frame(app);
                            app.installed_inputs.emplace(id);
                        }
                        i.install(app);
                        return nil::gate::edges::Compatible(i.input);
                    }()...
                );
            },
            inputs
        );
        const auto gate_outputs = app.gate.node(std::move(callable), gate_inputs);
        for_each(
            [&](auto& output, const auto& gate_output)
            {
                using output_t = std::decay_t<decltype(output)>;
                constexpr auto id = &Identity<output_t>::id;
                if (!app.installed_outputs.contains(id))
                {
                    app.installed_outputs.emplace(id, output_t::install_frame(app));
                }
                output.install(app, tag, app.get_installed_output<output_t>(), gate_output);
            },
            outputs,
            gate_outputs,
            std::make_index_sequence<sizeof...(Outputs)>()
        );
    }

    void* get_input(const void* id) override
    {
        void* result = nullptr;
        std::apply([&](auto&... pack) { (getter_impl(id, pack, &result) || ...); }, inputs);
        return result;
    }

    void* get_output(const void* id) override
    {
        void* result = nullptr;
        std::apply([&](auto&... pack) { (getter_impl(id, pack, &result) || ...); }, outputs);
        return result;
    }

    std::tuple<Inputs...> inputs;
    std::tuple<Outputs...> outputs;

private:
    template <typename T>
    static bool getter_impl(const void* id, T& i, void** output)
    {
        if (id == &Identity<T>::id)
        {
            *output = &i;
            return true;
        }
        return false;
    }
};

int main()
{
    const auto source_path = std::filesystem::path(__FILE__).parent_path();
    const auto http_server = nil::xit::make_server({
        .source_path = source_path.parent_path(),
        .port = 1101,
        .buffer_size = 1024ul * 1024ul * 100ul //
    });

    App app(use_ws(http_server, "/ws"));

    set_relative_directory(app.xit, source_path);
    set_cache_directory(app.xit, std::filesystem::temp_directory_path() / "nil-xit-gtest");

    auto& main_frame = add_unique_frame(app.xit, "demo", "gui/Main.svelte");
    add_value(main_frame, "scenes", nlohmann::json::parse(R"({ "scenes": ["", "a", "b"] })"));
    add_value(main_frame, "selected", 0L);

    app.gate.set_runner<nil::gate::runners::NonBlocking>();
    app.add_item<Item<std::tuple<InputFrame, SliderInputFrame>, std::tuple<OutputFrame>>>(
        Callable("a"),
        "a"
    );
    app.add_item<Item<std::tuple<InputFrame, SliderInputFrame>, std::tuple<OutputFrame>>>(
        Callable("b"),
        "b"
    );

    // TODO: input frame separate from unique/tagged
    // TODO: direct communication for each frame (avoid unnecessary broadcasting to all frames)

    on_ready(http_server, [&]() { app.gate.commit(); });
    start(http_server);
    return 0;
}
