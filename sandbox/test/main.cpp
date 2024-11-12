#include <filesystem>
#include <nil/xit.hpp>

#include <nil/gate.hpp>
#include <nil/gate/bias/nil.hpp>
#include <nil/gate/runners/NonBlocking.hpp>

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

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

namespace nil::xit::test
{
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

    template <typename P, typename A, typename B, std::size_t... I>
    void for_each(P predicate, const A& a, const B& b, std::index_sequence<I...> /* indices */)
    {
        (([&]() { predicate(get<I>(a), get<I>(b)); }()), ...);
    }

    struct RerunTag
    {
        bool operator==(const RerunTag& /* o */) const
        {
            return false;
        }
    };

    namespace frame
    {
        struct IInfo
        {
            IInfo() = default;
            IInfo(IInfo&&) = delete;
            IInfo(const IInfo&) = delete;
            IInfo& operator=(IInfo&&) = delete;
            IInfo& operator=(const IInfo&) = delete;
            virtual ~IInfo() = default;
        };

        namespace input
        {
            template <typename T>
            struct Info: IInfo
            {
                using type = T;

                virtual nil::gate::edges::Compatible<T> get_input(std::string_view tag) = 0;
                virtual void add_tag(std::string_view tag) = 0;
            };

            namespace unique
            {
                template <typename T>
                struct Info: input::Info<T>
                {
                    nil::xit::unique::Frame* frame = nullptr;
                    nil::gate::Core* gate = nullptr;
                    std::optional<T> data;
                    nil::gate::edges::Mutable<T>* input = nullptr;

                    nil::gate::edges::Compatible<T> get_input(std::string_view /* tag */) override
                    {
                        return input;
                    }

                    void add_tag(std::string_view /* tag */) override
                    {
                    }

                    template <typename V, typename Getter, typename Setter>
                        requires requires(Getter g, Setter s) {
                            { g(std::declval<const T&>()) } -> std::same_as<V>;
                            { s(std::declval<T&>(), std::declval<V>()) } -> std::same_as<void>;
                        }
                    void add_value(std::string id, Getter getter, Setter setter)
                    {
                        if (!data.has_value())
                        {
                            return;
                        }
                        nil::xit::unique::add_value(
                            *frame,
                            id,
                            getter(data.value()),
                            [this, setter = std::move(setter)](V new_data)
                            {
                                setter(data.value(), std::move(new_data));
                                input->set_value(data.value());
                                gate->commit();
                            }
                        );
                    }
                };
            }

            namespace tagged
            {
                template <typename T>
                struct Info: input::Info<T>
                {
                    struct Entry
                    {
                        std::optional<T> data;
                        nil::gate::edges::Mutable<T>* input = nullptr;
                    };

                    nil::xit::tagged::Frame* frame = nullptr;
                    nil::gate::Core* gate = nullptr;
                    std::function<T(std::string_view)> initializer;
                    transparent::hash_map<Entry> info;

                    nil::gate::edges::Compatible<T> get_input(std::string_view tag) override
                    {
                        if (const auto it = info.find(tag); it != info.end())
                        {
                            return it->second.input;
                        }
                        return nullptr;
                    }

                    void add_tag(std::string_view tag) override
                    {
                        info.emplace(tag, typename Info<T>::Entry{std::nullopt, gate->edge<T>()});
                    }

                    template <typename V, typename Getter, typename Setter>
                        requires requires(Getter g, Setter s) {
                            { g(std::declval<const T&>()) } -> std::same_as<V>;
                            { s(std::declval<T&>(), std::declval<V>()) } -> std::same_as<void>;
                        }
                    void add_value(std::string id, Getter getter, Setter setter)
                    {
                        nil::xit::tagged::add_value(
                            *frame,
                            id,
                            [this, getter = std::move(getter)](std::string_view tag)
                            {
                                if (auto it = info.find(tag); it != info.end())
                                {
                                    auto& entry = it->second;
                                    if (!entry.data.has_value())
                                    {
                                        entry.data = initializer(tag);
                                        entry.input->set_value(entry.data.value());
                                        gate->commit();
                                    }
                                    return getter(entry.data.value());
                                }
                                return V();
                            },
                            [this, setter = std::move(setter)](std::string_view tag, V new_data)
                            {
                                if (auto it = info.find(tag); it != info.end())
                                {
                                    auto& entry = it->second;
                                    setter(entry.data.value(), std::move(new_data));
                                    entry.input->set_value(entry.data.value());
                                    gate->commit();
                                }
                            }
                        );
                    }
                };
            }
        }

        namespace output
        {
            template <typename T>
            struct Info: IInfo
            {
                using type = T;
                transparent::hash_map<nil::gate::edges::Mutable<RerunTag>*> rerun;
                nil::xit::tagged::Value<T>* output = nullptr;
            };
        }
    }

    struct App
    {
        explicit App(nil::xit::C init_xit)
            : xit(std::move(init_xit))
        {
            gate.set_runner<nil::gate::runners::NonBlocking>();
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
            std::apply([&](auto*... input) { (input->add_tag(tag), ...); }, inputs);
            if constexpr (sizeof...(Outputs) == 0)
            {
                gate.node(
                    std::move(callable),
                    std::apply(
                        [&](auto*... i) { return std::make_tuple(i->get_input(tag)...); },
                        inputs
                    )
                );
            }
            else
            {
                auto gate_outputs = gate.node(
                    std::move(callable),
                    std::apply(
                        [&](auto*... i) { return std::make_tuple(i->get_input(tag)...); },
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
        frame::input::tagged::Info<T>* add_tagged_input(
            std::string id,
            std::filesystem::path path,
            std::function<T(std::string_view)> initializer
        )
        {
            auto* s = make_frame<frame::input::tagged::Info<T>>(id, input_frames);
            s->frame = &add_tagged_frame(xit, std::move(id), std::move(path));
            s->gate = &gate;
            s->initializer = std::move(initializer);
            return s;
        }

        template <typename T>
        frame::input::unique::Info<T>* add_unique_input(
            std::string id,
            std::filesystem::path path,
            T init_data
        )
        {
            auto* s = make_frame<frame::input::unique::Info<T>>(id, input_frames);
            s->frame = &add_unique_frame(xit, std::move(id), std::move(path));
            s->gate = &gate;
            s->input = gate.edge(init_data);
            s->data = std::move(init_data);
            return s;
        }

        template <typename T>
        frame::output::Info<T>* add_output(std::string id, std::filesystem::path path)
        {
            auto* s = make_frame<frame::output::Info<T>>(id, output_frames);
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

        template <typename T>
        frame::input::Info<T>* get_input(std::string_view id) const
        {
            if (auto it = input_frames.find(id); it != input_frames.end())
            {
                return static_cast<frame::input::Info<T>*>(it->second.get());
            }
            return nullptr;
        }

        template <typename T>
        frame::output::Info<T>* get_output(std::string_view id) const
        {
            if (auto it = output_frames.find(id); it != output_frames.end())
            {
                return static_cast<frame::output::Info<T>*>(it->second.get());
            }
            return nullptr;
        }

        nil::xit::C xit;
        nil::gate::Core gate;

        transparent::hash_map<std::unique_ptr<frame::IInfo>> input_frames;
        transparent::hash_map<std::unique_ptr<frame::IInfo>> output_frames;
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
    };

    template <typename... T>
    struct Inputs
    {
        std::tuple<const T*...> data;
    };

    template <typename... T>
    struct InputFrames;
    template <typename... T>
    struct OutputFrames;

    template <typename P, typename I, typename O>
    struct Base;

    template <typename P, typename... T, StringLiteral... I, typename... U, StringLiteral... O>
        requires(sizeof...(T) == sizeof...(I) && sizeof...(U) == sizeof...(O))
    struct Base<P, InputFrames<Frame<T, I>...>, OutputFrames<Frame<U, O>...>>
    {
        virtual ~Base() noexcept = default;
        Base() = default;
        Base(Base&&) = delete;
        Base(const Base&) = delete;
        Base& operator=(Base&&) = delete;
        Base& operator=(const Base&) = delete;

        using base_t = Base<P, InputFrames<Frame<T, I>...>, OutputFrames<Frame<U, O>...>>;
        using return_t = std::conditional_t<sizeof...(U) == 0, void, std::tuple<U...>>;
        using argument_t = Inputs<T...>;

        virtual void setup() {};
        virtual void teardown() {};
        virtual return_t run(argument_t args) = 0;
    };

    template <typename P, typename... T, StringLiteral... I, typename... U, StringLiteral... O>
    void install(
        type<Base<P, InputFrames<Frame<T, I>...>, OutputFrames<Frame<U, O>...>>> /* type */,
        App& a,
        std::string_view tag
    )
    {
        using base_t = Base<P, InputFrames<Frame<T, I>...>, OutputFrames<Frame<U, O>...>>;
        using return_t = base_t::return_t;
        using argument_t = base_t::argument_t;
        a.add_node(
            tag,
            [](const T&... args) -> return_t
            {
                P p;
                p.setup();
                return [&]()
                {
                    auto r = p.run(argument_t{{std::addressof(args)...}});
                    p.teardown();
                    return r;
                }();
            },
            std::make_tuple(a.get_input<T>(I.value)...), // NOLINT
            std::make_tuple(a.get_output<U>(O.value)...) // NOLINT
        );
    }

    namespace builders
    {
        struct IFrame
        {
            virtual ~IFrame() = default;
            IFrame() = default;
            IFrame(IFrame&&) = delete;
            IFrame(const IFrame&) = delete;
            IFrame& operator=(IFrame&&) = delete;
            IFrame& operator=(const IFrame&) = delete;
            virtual void install(App& app) = 0;
        };

        namespace input
        {
            template <typename Accessor, typename T>
            concept is_compatible_accessor = requires(const Accessor& accessor) {
                {
                    accessor.set(
                        std::declval<T&>(),
                        std::declval<decltype(accessor.get(std::declval<const T&>()))>()
                    )
                } -> std::same_as<void>;
            };
            template <typename Getter, typename Setter, typename T>
            concept is_compatible_getter_setter = requires(Getter getter, Setter setter) {
                {
                    setter(
                        std::declval<T&>(),
                        std::declval<decltype(getter(std::declval<const T&>()))>()
                    )
                } -> std::same_as<void>;
            };

            namespace tagged
            {
                template <typename T>
                struct Frame final: IFrame
                {
                    Frame(
                        std::string init_id,
                        std::filesystem::path init_file,
                        std::function<T(std::string_view)> init_initializer
                    )
                        : IFrame()
                        , id(std::move(init_id))
                        , file(std::move(init_file))
                        , initializer(std::move(init_initializer))
                    {
                    }

                    void install(App& app) override
                    {
                        auto* frame = app.add_tagged_input(id, file, initializer);
                        for (const auto& value_installer : values)
                        {
                            value_installer(*frame);
                        }
                    }

                    template <typename Getter, typename Setter>
                        requires(is_compatible_getter_setter<Getter, Setter, T>)
                    Frame<T>& value(std::string value_id, Getter getter, Setter setter)
                    {
                        using type
                            = std::remove_cvref_t<decltype(getter(std::declval<const T&>()))>;
                        values.emplace_back(
                            [value_id = std::move(value_id),
                             getter = std::move(getter),
                             setter = std::move(setter)](frame::input::tagged::Info<T>& info) {
                                info.template add_value<type>(
                                    value_id,
                                    std::move(getter),
                                    std::move(setter)
                                );
                            }
                        );
                        return *this;
                    }

                    template <is_compatible_accessor<T> Accessor>
                    Frame<T>& value(std::string value_id, Accessor accessor)
                    {
                        using type = decltype(accessor.get(std::declval<const T&>()));
                        return value(
                            std::move(value_id),
                            [accessor](const T& value) { return accessor.get(value); },
                            [accessor](T& value, type new_value)
                            { return accessor.set(value, std::move(new_value)); }
                        );
                    }

                    Frame<T>& value(std::string value_id)
                    {
                        return value(
                            std::move(value_id),
                            [](const T& value) { return value; },
                            [](T& value, T new_value) { value = std::move(new_value); }
                        );
                    }

                    std::string id;
                    std::filesystem::path file;
                    std::function<T(std::string_view)> initializer;
                    std::vector<std::function<void(frame::input::tagged::Info<T>&)>> values;
                };
            }

            namespace unique
            {
                template <typename T>
                struct Frame final: IFrame
                {
                    Frame(std::string init_id, std::filesystem::path init_file, T init_initializer)
                        : IFrame()
                        , id(std::move(init_id))
                        , file(std::move(init_file))
                        , initializer(std::move(init_initializer))
                    {
                    }

                    void install(App& app) override
                    {
                        auto* frame = app.add_unique_input(id, file, initializer);
                        for (const auto& value_installer : values)
                        {
                            value_installer(*frame);
                        }
                    }

                    template <typename Getter, typename Setter>
                        requires(is_compatible_getter_setter<Getter, Setter, T>)
                    Frame<T>& value(std::string value_id, Getter getter, Setter setter)
                    {
                        using type = decltype(getter(std::declval<const T&>()));
                        values.emplace_back(
                            [value_id = std::move(value_id),
                             getter = std::move(getter),
                             setter = std::move(setter)](frame::input::unique::Info<T>& info) {
                                info.template add_value<type>(
                                    value_id,
                                    std::move(getter),
                                    std::move(setter)
                                );
                            }
                        );
                        return *this;
                    }

                    template <is_compatible_accessor<T> Accessor>
                    Frame<T>& value(std::string value_id, Accessor accessor)
                    {
                        using type = decltype(accessor.get(std::declval<const T&>()));
                        return value(
                            std::move(value_id),
                            [accessor](const T& value) { return accessor.get(value); },
                            [accessor](T& value, type new_value)
                            { return accessor.set(value, std::move(new_value)); }
                        );
                    }

                    Frame<T>& value(std::string value_id)
                    {
                        return value(
                            std::move(value_id),
                            [](const T& value) { return value; },
                            [](T& value, T new_value) { value = std::move(new_value); }
                        );
                    }

                    std::string id;
                    std::filesystem::path file;
                    T initializer;
                    std::vector<std::function<void(frame::input::unique::Info<T>&)>> values;
                };
            }
        }

        namespace output
        {
            template <typename T>
            struct Frame final: IFrame
            {
                Frame(std::string init_id, std::filesystem::path init_file)
                    : IFrame()
                    , id(std::move(init_id))
                    , file(std::move(init_file))
                {
                }

                void install(App& app) override
                {
                    app.add_output<T>(id, file);
                }

                std::string id;
                std::filesystem::path file;
            };
        }

        struct FrameBuilder
        {
            template <typename Initializer>
            auto& create_tagged_input(
                std::string id,
                std::filesystem::path file,
                Initializer initializer
            )
            {
                using type = decltype(initializer(std::declval<std::string_view>()));
                auto ptr = std::make_unique<input::tagged::Frame<type>>(
                    std::move(id),
                    file,
                    std::move(initializer)
                );

                auto* raw_ptr = ptr.get();
                frames.emplace_back(std::move(ptr));
                return *raw_ptr;
            }

            template <typename T>
            auto& create_unique_input(std::string id, std::filesystem::path file, T init_value)
            {
                auto ptr = std::make_unique<input::unique::Frame<T>>(
                    std::move(id),
                    file,
                    std::move(init_value)
                );

                auto* raw_ptr = ptr.get();
                frames.emplace_back(std::move(ptr));
                return *raw_ptr;
            }

            template <typename T>
            void create_output(std::string id, std::filesystem::path file, test::type<T> /* type */)
            {
                frames.emplace_back(std::make_unique<output::Frame<T>>(std::move(id), file));
            }

            void install(App& app)
            {
                for (const auto& frame : frames)
                {
                    frame->install(app);
                }
            }

            std::vector<std::unique_ptr<IFrame>> frames;
        };
    }

    nlohmann::json as_json(std::istream& iss)
    {
        return nlohmann::json::parse(iss);
    }

    template <typename Reader>
        requires requires(Reader reader) {
            { reader(std::declval<std::istream&>()) };
        }
    auto from_file(std::filesystem::path source_path, std::string file_name, Reader reader)
    {
        using type = decltype(reader(std::declval<std::istream&>()));
        return [source_path = std::move(source_path),
                file_name = std::move(file_name),
                reader = std::move(reader)](std::string_view tag)
        {
            auto path = source_path / tag / file_name;
            if (!std::filesystem::exists(path))
            {
                std::cout << path << std::endl;
                std::cout << "not found" << std::endl;
                return type(); // TODO: throw? or default?
            }
            std::ifstream file(path, std::ios::binary);
            return reader(file);
        };
    }

    template <typename C, typename M>
    auto from_member(M C::*member_ptr)
    {
        struct Accessor
        {
            M get(const C& data) const
            {
                return data.*member_ptr;
            }

            void set(C& data, M new_data) const
            {
                data.*member_ptr = std::move(new_data);
            }

            M C::*member_ptr;
        };

        return Accessor{member_ptr};
    }

    template <typename T>
    auto from_json_ptr(const std::string& json_ptr)
    {
        struct Accessor
        {
            T get(const nlohmann::json& data) const
            {
                return data[json_ptr];
            }

            void set(nlohmann::json& data, T new_data) const
            {
                data[json_ptr] = std::move(new_data);
            }

            nlohmann::json::json_pointer json_ptr;
        };

        return Accessor{nlohmann::json::json_pointer(json_ptr)};
    }
}

template <typename... T>
struct std::tuple_size<nil::xit::test::Inputs<T...>>
    : std::integral_constant<std::size_t, sizeof...(T)>
{
};

template <std::size_t I, typename... T>
const auto& get(const nil::xit::test::Inputs<T...>& o)
{
    return *std::get<I>(o.data);
}

template <std::size_t I, typename... T>
struct std::tuple_element<I, nil::xit::test::Inputs<T...>>
{
    using type = decltype(*get<I>(std::declval<nil::xit::test::Inputs<T...>>().data));
};

struct Ranges
{
    std::int64_t v1;
    std::int64_t v2;
    std::int64_t v3;

    bool operator==(const Ranges& o) const
    {
        return v1 == o.v1 && v2 == o.v2 && v3 == o.v3;
    }
};

// This is the example of the "test"
struct Derived final
    : nil::xit::test::Base<
          Derived,
          nil::xit::test::InputFrames<
              nil::xit::test::Frame<nlohmann::json, "input_frame">, //
              nil::xit::test::Frame<Ranges, "slider_frame">>,       //
          nil::xit::test::OutputFrames<                             //
              nil::xit::test::Frame<nlohmann::json, "view_frame">   //
              >>
{
    return_t run(argument_t args) override;
};

// TODO: optionality of returning? pass as an output argument?
Derived::return_t Derived::run(Derived::argument_t args) // NOLINT
{
    const auto& [input_data, ranges] = args;
    auto tag = std::string(input_data["x"][2]);
    std::cout << "run (test) " << tag << std::endl;
    auto result = input_data;
    result["y"][0] = input_data["y"][0].get<std::int64_t>() * ranges.v1;
    result["y"][1] = input_data["y"][1].get<std::int64_t>() * ranges.v2;
    result["y"][2] = input_data["y"][2].get<std::int64_t>() * ranges.v3;
    return {result};
}

int main()
{
    using nil::xit::test::App;
    using nil::xit::test::as_json; // TODO how about write logic?
    using nil::xit::test::from_file;
    using nil::xit::test::from_member;
    using nil::xit::test::type;
    using nil::xit::test::builders::FrameBuilder;

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
    add_value(main_frame, "pane", nlohmann::json::parse(R"(["slider_frame", "input_frame"])"));

    FrameBuilder frame_builder;
    frame_builder
        .create_tagged_input(
            "input_frame",
            "gui/EditorFrame.svelte",
            nil::xit::test::from_file(source_path / "files", "input_frame.json", &as_json)
        )
        .value("value");
    frame_builder
        .create_unique_input("slider_frame", "gui/Slider.svelte", Ranges{0, 0, 0}) //
        .value("value-1", from_member(&Ranges::v1))
        .value("value-2", from_member(&Ranges::v2))
        .value("value-3", from_member(&Ranges::v3));
    // example using from_json_ptr
    // frame_builder
    //     .create_unique_input<nlohmann::json>(
    //         "slider_frame",
    //         "gui/Slider.svelte",
    //         nlohmann::json::array({0, 0, 0})
    //     ) //
    //     .value("value-1", from_json_ptr<std::int64_t>("/0"))
    //     .value("value-2", from_json_ptr<std::int64_t>("/1"))
    //     .value("value-3", from_json_ptr<std::int64_t>("/2"));
    frame_builder.create_output("view_frame", "gui/ViewFrame.svelte", type<nlohmann::json>());
    frame_builder.install(app);

    // TODO: store this somewhere to be called during registration
    install(type<Derived::base_t>(), app, "a");
    install(type<Derived::base_t>(), app, "b");

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
