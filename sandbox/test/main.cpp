#include <nil/xit.hpp>

#include <nil/gate.hpp>
#include <nil/gate/bias/nil.hpp>
#include <nil/gate/runners/NonBlocking.hpp>

#include <nlohmann/json.hpp>

#include <iostream>
#include <tuple>
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
void for_each(P predicate, A&& a, B&& b, std::index_sequence<I...> /* indices */)
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

template <typename T>
struct UniqueFrameInfo: IFrameInfo
{
    using type = T;
    nil::gate::edges::Mutable<T>* input = nullptr;

    nil::gate::edges::Compatible<T> get_input(std::string_view /* tag */)
    {
        return input;
    }
};

template <typename T>
struct TaggedFrameInfo: IFrameInfo
{
    using type = T;

    struct Entry
    {
        T data = T();
        nil::gate::edges::Mutable<T>* input = nullptr;
    };

    std::unordered_map<std::string, Entry, transparent::Hash, transparent::Equal> info;

    nil::gate::edges::Compatible<T> get_input(std::string_view tag)
    {
        if (const auto it = info.find(tag); it != info.end())
        {
            return it->second.input;
        }
        return nullptr;
    }
};

template <typename T>
struct OutputFrameInfo: IFrameInfo
{
    using type = T;
    std::unordered_map<
        std::string,
        nil::gate::edges::Mutable<RerunTag>*,
        transparent::Hash,
        transparent::Equal>
        rerun;
    nil::xit::tagged::Value<T>* output = nullptr;
};

template <typename T>
auto input(UniqueFrameInfo<T>* info)
{
    struct Input
    {
        UniqueFrameInfo<T>* info;

        void add_tag(std::string_view /* tag */, nil::gate::Core& /* gate */)
        {
        }
    };

    return Input{info};
}

template <typename T>
auto input(TaggedFrameInfo<T>* info, T value)
{
    struct Input
    {
        TaggedFrameInfo<T>* info;
        T value;

        void add_tag(std::string_view tag, nil::gate::Core& gate)
        {
            auto* edge = gate.edge(value);
            info->info.emplace(tag, typename TaggedFrameInfo<T>::Entry{std::move(value), edge});
        }
    };

    return Input{info, std::move(value)};
}

struct App2
{
    explicit App2(nil::service::S service)
        : xit(nil::xit::make_core(service))
    {
    }

    ~App2() noexcept = default;
    App2(App2&&) = delete;
    App2(const App2&) = delete;
    App2& operator=(App2&&) = delete;
    App2& operator=(const App2&) = delete;

    template <typename Callable, typename... I, typename... O>
    void add_node(
        std::string_view tag,
        Callable callable,
        std::tuple<I...> inputs,
        std::tuple<O*...> outputs
    )
    {
        apply([tag, this](auto&... input) { (input.add_tag(tag, this->gate), ...); }, inputs);
        auto gate_outputs = gate.node(
            std::move(callable),
            std::apply(
                [tag](auto&... i) { return std::make_tuple(i.info->get_input(tag)...); },
                inputs
            )
        );
        for_each(
            [&](auto* output, auto* gate_output)
            {
                using o_t = typename std::decay_t<std::remove_pointer_t<decltype(output)>>::type;
                auto& [key, rerun] = *output->rerun.emplace(tag, this->gate.edge(RerunTag())).first;
                gate.node(
                    [output, &key](RerunTag, const o_t& output_data)
                    {
                        std::cout << "out (test) " << key << std::endl;
                        post(key, *output->output, output_data);
                    },
                    {rerun, gate_output}
                );
            },
            outputs,
            gate_outputs,
            std::make_index_sequence<sizeof...(O)>()
        );
    }

    template <typename T>
    TaggedFrameInfo<T>* add_tagged_input(std::string id, std::filesystem::path path)
    {
        auto* s = make_frame<TaggedFrameInfo<T>>(id);
        auto& frame = add_tagged_frame(xit, std::move(id), std::move(path));
        add_value(
            frame,
            "value",
            [s](std::string_view tag)
            {
                if (auto it = s->info.find(tag); it != s->info.end())
                {
                    return it->second.data;
                }
                return T();
            },
            [s, g = &this->gate](std::string_view tag, T new_data)
            {
                if (auto it = s->info.find(tag); it != s->info.end())
                {
                    it->second.data = std::move(new_data);
                    it->second.input->set_value(it->second.data);
                    g->commit();
                }
            }
        );
        return s;
    }

    template <typename T>
    UniqueFrameInfo<T>* add_unique_input(std::string id, std::filesystem::path path, T init_data)
    {
        auto* s = make_frame<UniqueFrameInfo<T>>(id);
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
    OutputFrameInfo<T>* add_output(std::string id, std::filesystem::path path)
    {
        auto* s = make_frame<OutputFrameInfo<T>>(id);
        auto& frame = add_tagged_frame(
            xit,
            std::move(id),
            std::move(path),
            [s, g = &gate](std::string_view tag)
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
    T* make_frame(std::string_view tag)
    {
        auto t = std::make_unique<T>();
        auto p = t.get();
        frames.emplace(tag, std::move(t));
        return p;
    }

    nil::xit::C xit;
    nil::gate::Core gate;

    std::unordered_map<std::string, std::unique_ptr<IFrameInfo>> frames;
};

int main()
{
    const auto source_path = std::filesystem::path(__FILE__).parent_path();
    const auto http_server = nil::xit::make_server({
        .source_path = source_path.parent_path(),
        .port = 1101,
        .buffer_size = 1024ul * 1024ul * 100ul //
    });

    App2 app(use_ws(http_server, "/ws"));

    set_relative_directory(app.xit, source_path);
    set_cache_directory(app.xit, std::filesystem::temp_directory_path() / "nil-xit-gtest");

    auto& main_frame = add_unique_frame(app.xit, "demo", "gui/Main.svelte");
    add_value(main_frame, "tags", nlohmann::json::parse(R"(["", "a", "b"])"));
    add_value(main_frame, "view", nlohmann::json::parse(R"(["view_frame"])"));
    add_value(main_frame, "pane", nlohmann::json::parse(R"(["editor_frame"])"));
    add_value(main_frame, "selected", 1L);

    app.gate.set_runner<nil::gate::runners::NonBlocking>();
    auto* editor = app.add_tagged_input<nlohmann::json>("editor_frame", "gui/EditorFrame.svelte");
    auto* slider = app.add_unique_input<std::int64_t>("slider_frame", "gui/Slider.svelte", 0L);
    auto* view = app.add_output<nlohmann::json>("view_frame", "gui/ViewFrame.svelte");

    const auto data = []()
    {
        return nlohmann::json::array({nlohmann::json::object(
            {{"x", nlohmann::json::array({"giraffes", "orangutans", "monkeys"})},
             {"y", nlohmann::json::array({20, 14, 23})},
             {"type", "bar"}}
        )});
    };

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

    app.add_node(
        "a",
        Callable("a"),
        std::make_tuple(input(editor, data()), input(slider)),
        std::make_tuple(view)
    );
    app.add_node(
        "b",
        Callable("b"),
        std::make_tuple(input(editor, data()), input(slider)),
        std::make_tuple(view)
    );

    // TODO: direct communication for each frame (avoid unnecessary broadcasting to all frames)
    // TODO: how about when input/output are type erased. how do check compatibility with the
    // node/callable

    on_ready(http_server, [&]() { app.gate.commit(); });
    start(http_server);
    return 0;
}