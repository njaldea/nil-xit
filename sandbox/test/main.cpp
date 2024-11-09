#include <nil/xit.hpp>

#include <nil/gate.hpp>
#include <nil/gate/bias/nil.hpp>
#include <nil/gate/runners/NonBlocking.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
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

    template <typename T>
    using hash_map = std::unordered_map<std::string, T, Hash, Equal>;
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

template <typename P, typename A, typename B, std::size_t... I>
void for_each(P predicate, const A& a, const B& b, std::index_sequence<I...> /* indices */)
{
    (([&]() { predicate(get<I>(a), get<I>(b)); }()), ...);
}

struct IFrameInfo
{
    IFrameInfo() = default;
    IFrameInfo(IFrameInfo&&) = delete;
    IFrameInfo(const IFrameInfo&) = delete;
    IFrameInfo& operator=(IFrameInfo&&) = delete;
    IFrameInfo& operator=(const IFrameInfo&) = delete;
    virtual ~IFrameInfo() = default;
};

namespace input
{

    template <typename T>
    struct FrameInfo: IFrameInfo
    {
        using type = T;

        FrameInfo() = default;
        FrameInfo(FrameInfo&&) = delete;
        FrameInfo(const FrameInfo&) = delete;
        FrameInfo& operator=(FrameInfo&&) = delete;
        FrameInfo& operator=(const FrameInfo&) = delete;
        ~FrameInfo() override = default;

        virtual nil::gate::edges::Compatible<T> get_input(std::string_view tag) = 0;
        virtual void add_tag(std::string_view tag, nil::gate::Core& gate) = 0;
    };

    template <typename T>
    struct UniqueFrameInfo: FrameInfo<T>
    {
        nil::gate::edges::Mutable<T>* input = nullptr;

        nil::gate::edges::Compatible<T> get_input(std::string_view /* tag */) override
        {
            return input;
        }

        void add_tag(std::string_view /* tag */, nil::gate::Core& /* gate */) override
        {
        }
    };

    template <typename T>
    struct TaggedFrameInfo: FrameInfo<T>
    {
        struct Entry
        {
            std::optional<T> data;
            nil::gate::edges::Mutable<T>* input = nullptr;
        };

        transparent::hash_map<Entry> info;

        nil::gate::edges::Compatible<T> get_input(std::string_view tag) override
        {
            if (const auto it = info.find(tag); it != info.end())
            {
                return it->second.input;
            }
            return nullptr;
        }

        void add_tag(std::string_view tag, nil::gate::Core& gate) override
        {
            info.emplace(tag, typename TaggedFrameInfo<T>::Entry{std::nullopt, gate.edge<T>()});
        }
    };
}

namespace output
{
    template <typename T>
    struct FrameInfo: IFrameInfo
    {
        using type = T;
        transparent::hash_map<nil::gate::edges::Mutable<RerunTag>*> rerun;
        nil::xit::tagged::Value<T>* output = nullptr;
    };
}

struct App
{
    explicit App(nil::xit::C init_xit)
        : xit(std::move(init_xit))
    {
    }

    ~App() noexcept = default;
    App(App&&) = delete;
    App(const App&) = delete;
    App& operator=(App&&) = delete;
    App& operator=(const App&) = delete;

    template <typename Callable, typename... Inputs, typename... Outputs>
    void add_node(
        std::string_view tag,
        Callable callable,
        std::tuple<Inputs...> inputs,
        std::tuple<Outputs...> outputs
    )
    {
        apply([tag, this](auto*... input) { (input->add_tag(tag, this->gate), ...); }, inputs);
        if constexpr (sizeof...(Outputs) == 0)
        {
            gate.node(
                std::move(callable),
                std::apply(
                    [tag](auto*... i) { return std::make_tuple(i->get_input(tag)...); },
                    inputs
                )
            );
        }
        else
        {
            auto gate_outputs = gate.node(
                std::move(callable),
                std::apply(
                    [tag](auto*... i) { return std::make_tuple(i->get_input(tag)...); },
                    inputs
                )
            );
            for_each(
                [&](auto* output, auto* gate_output)
                {
                    using o_t =
                        typename std::decay_t<std::remove_pointer_t<decltype(output)>>::type;
                    auto& [key, rerun]
                        = *output->rerun.emplace(tag, this->gate.edge(RerunTag())).first;
                    gate.node(
                        [output, t = &key](RerunTag, const o_t& output_data)
                        {
                            std::cout << "out (test) " << *t << std::endl;
                            post(*t, *output->output, output_data);
                        },
                        {rerun, gate_output}
                    );
                },
                outputs,
                gate_outputs,
                std::make_index_sequence<sizeof...(Outputs)>()
            );
        }
    }

    template <typename T>
    input::TaggedFrameInfo<T>* add_tagged_input(
        std::string id,
        std::filesystem::path path,
        std::function<T(std::string_view)> initializer
    )
    {
        auto* s = make_frame<input::TaggedFrameInfo<T>>(id, input_frames);
        auto& frame = add_tagged_frame(xit, std::move(id), std::move(path));
        add_value(
            frame,
            "value",
            [s, initializer, g = &this->gate](std::string_view tag)
            {
                if (auto it = s->info.find(tag); it != s->info.end())
                {
                    auto& info = it->second;
                    if (!info.data.has_value())
                    {
                        info.data = initializer(tag);
                        info.input->set_value(info.data.value());
                        g->commit();
                    }
                    return info.data.value();
                }
                return T();
            },
            [s, g = &this->gate](std::string_view tag, T new_data)
            {
                if (auto it = s->info.find(tag); it != s->info.end())
                {
                    auto& info = it->second;
                    info.data = std::move(new_data);
                    info.input->set_value(info.data.value());
                    g->commit();
                }
            }
        );
        return s;
    }

    template <typename T>
    input::UniqueFrameInfo<T>* add_unique_input(
        std::string id,
        std::filesystem::path path,
        T init_data
    )
    {
        auto* s = make_frame<input::UniqueFrameInfo<T>>(id, input_frames);
        s->input = gate.edge(init_data);
        auto& frame = add_unique_frame(xit, std::move(id), std::move(path));
        add_value(
            frame,
            "value",
            std::move(init_data),
            [s, g = &this->gate](T new_data)
            {
                s->input->set_value(std::move(new_data));
                g->commit();
            }
        );
        return s;
    }

    template <typename T>
    output::FrameInfo<T>* add_output(std::string id, std::filesystem::path path)
    {
        auto* s = make_frame<output::FrameInfo<T>>(id, output_frames);
        auto& frame = add_tagged_frame(
            xit,
            std::move(id),
            std::move(path),
            [s, g = &this->gate](std::string_view tag)
            {
                if (const auto it = s->rerun.find(tag); it != s->rerun.end())
                {
                    it->second->set_value({});
                    g->commit();
                }
            }
        );
        s->output = &add_value(frame, "value", [](std::string_view /* tag */) { return T(); });
        return s;
    }

    template <typename T>
    T* make_frame(std::string_view id, auto& frames)
    {
        auto t = std::make_unique<T>();
        auto p = t.get();
        frames.emplace(id, std::move(t));
        return p;
    }

    nil::xit::C xit;
    nil::gate::Core gate;

    std::unordered_map<std::string, std::unique_ptr<IFrameInfo>> input_frames;
    std::unordered_map<std::string, std::unique_ptr<IFrameInfo>> output_frames;
};

template <typename... T>
struct type
{
};

template <size_t N>
struct StringLiteral
{
    // NOLINTNEXTLINE
    constexpr StringLiteral(const char (&str)[N])
    {
        // NOLINTNEXTLINE
        std::copy_n(str, N, value);
    }

    // NOLINTNEXTLINE
    char value[N];
};

template <typename T, StringLiteral S>
struct Frame
{
    using type = T;
};

template <typename... T>
struct Inputs
{
    std::tuple<const T*...> data;
};

template <typename... T>
struct std::tuple_size<Inputs<T...>>: std::integral_constant<std::size_t, sizeof...(T)>
{
};

template <std::size_t I, typename... T>
const auto& get(const Inputs<T...>& o)
{
    return *std::get<I>(o.data);
}

template <std::size_t I, typename... T>
struct std::tuple_element<I, Inputs<T...>>
{
    using type = decltype(*get<I>(std::declval<Inputs<T...>>().data));
};

template <typename P, typename I, typename O>
struct Base;

template <typename P, typename... T, StringLiteral... I, typename... U, StringLiteral... O>
struct Base<P, std::tuple<Frame<T, I>...>, std::tuple<Frame<U, O>...>>
{
    virtual ~Base() noexcept = default;
    Base() = default;
    Base(Base&&) = delete;
    Base(const Base&) = delete;
    Base& operator=(Base&&) = delete;
    Base& operator=(const Base&) = delete;

    static void install(App& a, std::string_view tag)
    {
        a.add_node(
            tag,
            [](const T&... args) -> return_t
            {
                P p;
                return p.run(argument_t{{std::addressof(args)...}});
            },
            std::make_tuple( // NOLINTNEXTLINE
                static_cast<input::FrameInfo<T>*>(a.input_frames.at(std::string(I.value)).get())...
            ),
            std::make_tuple( // NOLINTNEXTLINE
                static_cast<output::FrameInfo<U>*>(a.output_frames.at(std::string(O.value)).get()
                )...
            )
        );
    }

    using return_t = std::conditional_t<sizeof...(U) == 0, void, std::tuple<U...>>;
    using argument_t = Inputs<T...>;
};

struct Derived
    : Base<
          Derived,
          std::tuple<Frame<nlohmann::json, "editor_frame">, Frame<std::int64_t, "slider_frame">>,
          std::tuple<Frame<nlohmann::json, "view_frame">> //
          >
{
    Derived() = default;
    return_t run(argument_t args);
};

// TODO: optionality of returning? pass as an output argument?
Derived::return_t Derived::run(Derived::argument_t args) // NOLINT
{
    const auto& [input_data, value] = args;
    auto tag = std::string(input_data[0]["x"][2]);
    std::cout << "run (test) " << tag << std::endl;
    auto result = input_data;
    if (tag == "a")
    {
        result[0]["y"][0] = input_data[0]["y"][0].get<std::int64_t>() * value;
    }
    else if (tag == "b")
    {
        result[0]["y"][1] = input_data[0]["y"][1].get<std::int64_t>() * value;
    }
    return {result};
}

int main()
{
    const auto source_path = std::filesystem::path(__FILE__).parent_path();
    const auto http_server = nil::xit::make_server({
        .source_path = source_path.parent_path() / "node_modules/@nil-/xit",
        .port = 1101,
        .buffer_size = 1024ul * 1024ul * 100ul //
    });

    App app(nil::xit::make_core(http_server));

    set_relative_directory(app.xit, source_path);
    set_cache_directory(app.xit, std::filesystem::temp_directory_path() / "nil-xit-gtest");

    auto& main_frame = add_unique_frame(app.xit, "demo", "gui/Main.svelte");
    add_value(main_frame, "tags", nlohmann::json::parse(R"(["", "a", "b"])"));
    add_value(main_frame, "view", nlohmann::json::parse(R"(["view_frame"])"));
    add_value(main_frame, "pane", nlohmann::json::parse(R"(["slider_frame", "editor_frame"])"));

    app.gate.set_runner<nil::gate::runners::NonBlocking>();
    app.add_tagged_input<nlohmann::json>(
        "editor_frame",
        "gui/EditorFrame.svelte",
        [](std::string_view tag)
        {
            // TODO: move this to the frame itself
            // add trait to be able to read from and write to file
            return nlohmann::json::array({nlohmann::json::object(
                {{"x", nlohmann::json::array({"giraffes", "orangutans", tag})},
                 {"y", nlohmann::json::array({20, 14, 23})},
                 {"type", "bar"}}
            )});
        }
    );
    app.add_unique_input<std::int64_t>("slider_frame", "gui/Slider.svelte", 0L);
    app.add_output<nlohmann::json>("view_frame", "gui/ViewFrame.svelte");

    // TODO: store this somewhere to be called during registration
    Derived::install(app, "a");
    Derived::install(app, "b");

    // TODO:
    //  - test api for registering the input/output frames
    //  - test api for registering the node (test)
    //  - type erasure due to runtime storage
    //      - resolve in runtime if the frame is compatible to the test
    //  - value (binding) restructuring
    //      - only 1 value per frame is going to be forwarded to the test
    //      - how about having multiple value bindings?
    //          - having json path or id to collect into one json/struct?

    on_ready(http_server, [&]() { app.gate.commit(); });
    start(http_server);
    return 0;
}
