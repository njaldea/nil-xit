#pragma once

#include <nil/xit.hpp>

#include <nil/gate.hpp>
#include <nil/gate/bias/nil.hpp>
#include <nil/gate/runners/NonBlocking.hpp>

#include <string_view>
#include <unordered_map>

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
                virtual void init_input(std::string_view tag) = 0;
            };

            namespace unique
            {
                template <typename T>
                struct Info: input::Info<T>
                {
                    nil::xit::unique::Frame* frame = nullptr;
                    nil::gate::Core* gate = nullptr;
                    std::function<T()> initializer;

                    struct
                    {
                        std::optional<T> data;
                        nil::gate::edges::Mutable<T>* input = nullptr;
                    } info;

                    nil::gate::edges::Compatible<T> get_input(std::string_view /* tag */) override
                    {
                        return info.input;
                    }

                    void init_input(std::string_view /* tag */) override
                    {
                        if (info.input == nullptr)
                        {
                            info.input = gate->edge<T>();
                        }
                    }

                    template <typename V, typename Getter, typename Setter>
                        requires requires(Getter g, Setter s) {
                            { g(std::declval<const T&>()) } -> std::same_as<V>;
                            { s(std::declval<T&>(), std::declval<V>()) } -> std::same_as<void>;
                        }
                    void add_value(std::string id, Getter getter, Setter setter)
                    {
                        nil::xit::unique::add_value(
                            *frame,
                            id,
                            [this, getter = std::move(getter)]()
                            {
                                if (!info.data.has_value())
                                {
                                    info.data = this->initializer();
                                    info.input->set_value(info.data.value());
                                    gate->commit();
                                }
                                return getter(info.data.value());
                            },
                            [this, setter = std::move(setter)](V new_data)
                            {
                                setter(info.data.value(), std::move(new_data));
                                info.input->set_value(info.data.value());
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

                    void init_input(std::string_view tag) override
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
                nil::xit::tagged::Frame* frame = nullptr;
                transparent::hash_map<nil::gate::edges::Mutable<RerunTag>*> rerun;
                std::vector<std::function<void(std::string_view, const T&)>> values;

                template <typename V, typename Getter>
                    requires requires(Getter g) {
                        { g(std::declval<const T&>()) } -> std::same_as<V>;
                    }
                void add_value(std::string id, Getter getter)
                {
                    auto* value = &nil::xit::tagged::add_value(
                        *frame,
                        id,
                        [](std::string_view /* tag */) { return V(); }
                    );
                    values.push_back( //
                        [value, getter = std::move(getter)](std::string_view tag, const T& data)
                        { nil::xit::tagged::post(tag, *value, getter(data)); }
                    );
                }
            };
        }
    }

    template <typename... T>
        requires(std::is_same_v<T, std::remove_cvref_t<T>> && ...)
    struct type
    {
    };

    template <size_t N>
    struct StringLiteral
    {
        // NOLINTNEXTLINE
        constexpr StringLiteral(const char (&str)[N])
        {
            std::copy_n(&str[0], N, &value[0]);
        }

        // NOLINTNEXTLINE
        char value[N];
    };

    template <StringLiteral S, typename... T>
    struct Frame;

    template <StringLiteral S, typename T>
    struct Frame<S, T>
    {
        using type = T;
        using decayed_t = Frame<S, T>;
    };

    template <typename... T>
    struct InputFrames;
    template <typename... T>
    struct OutputFrames;

    template <typename... T>
    struct InputData
    {
        std::tuple<const T* const...> data;
    };

    template <typename... T>
    struct OutputData
    {
        std::tuple<T* const...> data;
    };

    template <std::size_t I, typename... T>
        requires(I < sizeof...(T))
    const auto& get(const InputData<T...>& o)
    {
        return *std::get<I>(o.data);
    }

    template <std::size_t I, typename... T>
        requires(I < sizeof...(T))
    auto& get(OutputData<T...>& o)
    {
        return *std::get<I>(o.data);
    }

    template <typename I, typename O>
    struct Test;

    template <typename... I, typename... O>
    struct Test<InputFrames<I...>, OutputFrames<O...>>
    {
        Test() = default;
        virtual ~Test() noexcept = default;
        Test(Test&&) = delete;
        Test(const Test&) = delete;
        Test& operator=(Test&&) = delete;
        Test& operator=(const Test&) = delete;

        using base_t = Test<InputFrames<I...>, OutputFrames<O...>>;
        using inputs_t = InputData<typename I::type...>;
        using outputs_t = OutputData<typename O::type...>;

        virtual void setup() {};
        virtual void teardown() {};
        virtual void run(const inputs_t& xit_inputs, outputs_t& xit_outputs) = 0;
    };

    template <StringLiteral... T>
    struct Input;
    template <StringLiteral... T>
    struct Output;

    template <StringLiteral... I, StringLiteral... O>
    struct Test<Input<I...>, Output<O...>>
        : Test<InputFrames<Frame<I>...>, OutputFrames<Frame<O>...>>
    {
    };

    class App
    {
    public:
        App(nil::service::S& service, std::string_view app_name)
            : xit(nil::xit::make_core(service))
        {
            gate.set_runner<nil::gate::runners::NonBlocking>();
            on_ready(service, [this]() { gate.commit(); });
            set_cache_directory(xit, std::filesystem::temp_directory_path() / app_name);
        }

        App(nil::service::HTTPService& service, std::string_view app_name)
            : xit(nil::xit::make_core(service))
        {
            gate.set_runner<nil::gate::runners::NonBlocking>();
            on_ready(service, [this]() { gate.commit(); });
            set_cache_directory(xit, std::filesystem::temp_directory_path() / app_name);
        }

        ~App() noexcept = default;
        App(App&&) = delete;
        App(const App&) = delete;
        App& operator=(App&&) = delete;
        App& operator=(const App&) = delete;

        const std::vector<std::string>& installed_tags() const
        {
            return tags;
        }

        template <typename P, typename... I, typename... O>
        void install(
            std::string_view tag,
            type<Test<InputFrames<I...>, OutputFrames<O...>>> /* type */
        )
        {
            tags.emplace_back(tag);
            using base_t = Test<InputFrames<I...>, OutputFrames<O...>>;
            add_node(
                tag,
                [](const typename I::type&... args) -> std::tuple<typename O::type...>
                {
                    using inputs_t = typename base_t::inputs_t;
                    using outputs_t = typename base_t::outputs_t;
                    std::tuple<typename O::type...> result;
                    try
                    {
                        P p;
                        p.setup();
                        auto inputs = inputs_t{{&args...}};
                        auto outputs
                            = std::apply([](auto&... o) { return outputs_t{{&o...}}; }, result);
                        p.run(inputs, outputs);
                        p.teardown();
                    }
                    catch (const std::exception&)
                    {
                    }
                    catch (...)
                    {
                    }
                    return result;
                },
                std::make_tuple(get_input(type<typename I::decayed_t>())...), // NOLINT
                std::make_tuple(get_output(type<typename O::decayed_t>())...) // NOLINT
            );
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
            std::function<T()> initializer
        )
        {
            auto* s = make_frame<frame::input::unique::Info<T>>(id, input_frames);
            s->frame = &add_unique_frame(xit, std::move(id), std::move(path));
            s->gate = &gate;
            s->initializer = std::move(initializer);
            return s;
        }

        template <typename T>
        frame::output::Info<T>* add_output(std::string id, std::filesystem::path path)
        {
            auto* s = make_frame<frame::output::Info<T>>(id, output_frames);
            s->frame = &add_tagged_frame(
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
            return s;
        }

    private:
        template <typename Callable, typename... Inputs>
        void add_node(
            std::string_view tag,
            Callable callable,
            std::tuple<Inputs...> inputs,
            std::tuple<> /* outputs */
        )
        {
            std::apply([&](auto*... input) { (input->add_tag(tag), ...); }, inputs);
            gate.node(
                std::move(callable),
                std::apply(
                    [&](auto*... i) { return std::make_tuple(i->get_input(tag)...); },
                    inputs
                )
            );
        }

        template <typename Callable, typename... Inputs, typename... Outputs>
        void add_node(
            std::string_view tag,
            Callable callable,
            std::tuple<Inputs...> inputs,
            std::tuple<Outputs...> outputs
        )
        {
            std::apply([&](auto*... input) { (input->init_input(tag), ...); }, inputs);
            auto gate_outputs = gate.node(
                std::move(callable),
                std::apply(
                    [&](auto*... i) { return std::make_tuple(i->get_input(tag)...); },
                    inputs
                )
            );
            [&]<std::size_t... I>(std::index_sequence<I...>)
            {
                (([&](){
                    auto* output = std::get<I>(outputs);
                    using info_t = std::remove_cvref_t<decltype(*output)>;
                    using output_t = info_t::type;
                    const auto& [key, rerun]
                        = *output->rerun.emplace(tag, gate.edge(RerunTag())).first;
                    gate.node(
                        [output, t = &key](RerunTag, const output_t& output_data)
                        {
                            for (const auto& value : output->values)
                            {
                                value(*t, output_data);
                            }
                        },
                        {rerun, get<I>(gate_outputs)}
                    );
                    })(), ...);
            }(std::make_index_sequence<sizeof...(Outputs)>());
        }

        template <typename T>
        T* make_frame(std::string_view id, auto& frames)
        {
            auto t = std::make_unique<T>();
            auto p = t.get();
            frames.emplace(id, std::move(t));
            return p;
        }

        template <typename T, StringLiteral S>
        frame::input::Info<T>* get_input(type<Frame<S, T>> /* type */) const
        {
            if (auto it = input_frames.find(std::string_view(&S.value[0]));
                it != input_frames.end())
            {
                return static_cast<frame::input::Info<T>*>(it->second.get());
            }
            return nullptr;
        }

        template <typename T, StringLiteral S>
        frame::output::Info<T>* get_output(type<Frame<S, T>> /* type */) const
        {
            if (auto it = output_frames.find(std::string_view(&S.value[0]));
                it != output_frames.end())
            {
                return static_cast<frame::output::Info<T>*>(it->second.get());
            }
            return nullptr;
        }

    public:
        nil::xit::C xit; // NOLINT

    private:
        nil::gate::Core gate;

        transparent::hash_map<std::unique_ptr<frame::IInfo>> input_frames;
        transparent::hash_map<std::unique_ptr<frame::IInfo>> output_frames;

        std::vector<std::string> tags;
    };

    template <typename T>
    auto from_data(T data)
    {
        struct Loader final
        {
        public:
            explicit Loader(T init_data)
                : data(std::move(init_data))
            {
            }

            ~Loader() noexcept = default;
            Loader(const Loader&) = default;
            Loader(Loader&&) = default;
            Loader& operator=(const Loader&) = default;
            Loader& operator=(Loader&&) = default;

            auto operator()(std::string_view /* tag */) const
            {
                return data;
            }

            auto operator()() const
            {
                return data;
            }

        private:
            T data;
        };

        return Loader{std::move(data)};
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

    namespace builders
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

        struct IFrame
        {
            virtual ~IFrame() = default;
            IFrame() = default;
            IFrame(IFrame&&) = delete;
            IFrame(const IFrame&) = delete;
            IFrame& operator=(IFrame&&) = delete;
            IFrame& operator=(const IFrame&) = delete;
            virtual void install(App& app, const std::filesystem::path& path) = 0;
        };

        namespace input
        {
            namespace tagged
            {
                template <typename T>
                struct Frame final: IFrame
                {
                    using type = T;

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

                    void install(App& app, const std::filesystem::path& path) override
                    {
                        auto* frame = app.add_tagged_input(id, path / file, initializer);
                        for (const auto& value_installer : values)
                        {
                            value_installer(*frame);
                        }
                    }

                    template <typename Getter, typename Setter>
                        requires(is_compatible_getter_setter<Getter, Setter, T>)
                    Frame<T>& value(std::string value_id, Getter getter, Setter setter)
                    {
                        using getter_return_t
                            = std::remove_cvref_t<decltype(getter(std::declval<const T&>()))>;
                        values.emplace_back(
                            [value_id = std::move(value_id),
                             getter = std::move(getter),
                             setter = std::move(setter)](frame::input::tagged::Info<T>& info) {
                                info.template add_value<getter_return_t>(
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
                        using getter_return_t = decltype(accessor.get(std::declval<const T&>()));
                        return value(
                            std::move(value_id),
                            [accessor](const T& value) { return accessor.get(value); },
                            [accessor](T& value, getter_return_t new_value)
                            { return accessor.set(value, std::move(new_value)); }
                        );
                    }

                    template <typename U>
                    Frame<T>& value(std::string value_id, U T::*member)
                    {
                        return value(std::move(value_id), from_member(member));
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
                    using type = T;

                    Frame(
                        std::string init_id,
                        std::filesystem::path init_file,
                        std::function<T()> init_initializer
                    )
                        : IFrame()
                        , id(std::move(init_id))
                        , file(std::move(init_file))
                        , initializer(std::move(init_initializer))
                    {
                    }

                    void install(App& app, const std::filesystem::path& path) override
                    {
                        auto* frame = app.add_unique_input(id, path / file, initializer);
                        for (const auto& value_installer : values)
                        {
                            value_installer(*frame);
                        }
                    }

                    template <typename Getter, typename Setter>
                        requires(is_compatible_getter_setter<Getter, Setter, T>)
                    Frame<T>& value(std::string value_id, Getter getter, Setter setter)
                    {
                        using getter_return_t = decltype(getter(std::declval<const T&>()));
                        values.emplace_back(
                            [value_id = std::move(value_id),
                             getter = std::move(getter),
                             setter = std::move(setter)](frame::input::unique::Info<T>& info) {
                                info.template add_value<getter_return_t>(
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
                        using getter_return_t = decltype(accessor.get(std::declval<const T&>()));
                        return value(
                            std::move(value_id),
                            [accessor](const T& value) { return accessor.get(value); },
                            [accessor](T& value, getter_return_t new_value)
                            { return accessor.set(value, std::move(new_value)); }
                        );
                    }

                    template <typename U>
                    Frame<T>& value(std::string value_id, U T::*member)
                    {
                        return value(std::move(value_id), from_member(member));
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
                    std::function<T()> initializer;
                    std::vector<std::function<void(frame::input::unique::Info<T>&)>> values;
                };
            }
        }

        namespace output
        {
            template <typename T>
            struct Frame final: IFrame
            {
                using type = T;

                Frame(std::string init_id, std::filesystem::path init_file)
                    : IFrame()
                    , id(std::move(init_id))
                    , file(std::move(init_file))
                {
                }

                void install(App& app, const std::filesystem::path& path) override
                {
                    auto* frame = app.add_output<T>(id, path / file);
                    for (const auto& value_installer : values)
                    {
                        value_installer(*frame);
                    }
                }

                template <typename Getter>
                    requires requires(Getter getter) {
                        { getter(std::declval<T>()) };
                    }
                Frame<T>& value(std::string value_id, Getter getter)
                {
                    using getter_return_t
                        = std::remove_cvref_t<decltype(getter(std::declval<const T&>()))>;
                    values.emplace_back(
                        [value_id = std::move(value_id),
                         getter = std::move(getter)](frame::output::Info<T>& info)
                        { info.template add_value<getter_return_t>(value_id, std::move(getter)); }
                    );
                    return *this;
                }

                template <typename Accessor>
                    requires requires(Accessor accessor) {
                        { accessor.get(std::declval<T>()) };
                    }
                Frame<T>& value(std::string value_id, Accessor accessor)
                {
                    return value(
                        std::move(value_id),
                        [accessor](const T& value) { return accessor.get(value); }
                    );
                }

                template <typename U>
                Frame<T>& value(std::string value_id, U T::*member)
                {
                    return value(std::move(value_id), from_member(member));
                }

                Frame<T>& value(std::string value_id)
                {
                    return value(std::move(value_id), [](const T& value) { return value; });
                }

                std::string id;
                std::filesystem::path file;
                std::vector<std::function<void(frame::output::Info<T>&)>> values;
            };
        }

        namespace main
        {
            template <typename T>
            struct Frame final: IFrame
            {
                explicit Frame(std::filesystem::path init_file, T init_converter)
                    : IFrame()
                    , file(std::move(init_file))
                    , converter(std::move(init_converter))
                {
                }

                void install(App& app, const std::filesystem::path& path) override
                {
                    auto& f = add_unique_frame(app.xit, "demo", path / file);
                    add_value(f, "tags", from_data(converter(app.installed_tags())));
                    add_value(f, "outputs", from_data(converter({"view_frame"})));
                    add_value(f, "inputs", from_data(converter({"slider_frame", "input_frame"})));
                }

                std::filesystem::path file;
                T converter;
            };
        }

        template <typename T, typename... Args>
        concept is_initializer = requires(T initializer) {
            { initializer(std::declval<Args>()...) };
        };

        struct MainBuilder
        {
            template <typename FromVS>
                requires requires(FromVS converter) {
                    { converter(std::declval<std::vector<std::string>>()) };
                }
            void create_main(std::filesystem::path file, FromVS converter)
            {
                frame = std::make_unique<main::Frame<FromVS>>(
                    std::move(file),
                    std::move(std::move(converter))
                );
            }

            void install(App& app, const std::filesystem::path& path) const
            {
                if (frame)
                {
                    frame->install(app, path);
                }
            }

            std::unique_ptr<IFrame> frame;
        };

        struct FrameBuilder
        {
            template <typename Initializer>
                requires(!is_initializer<Initializer, std::string_view>)
            auto& create_tagged_input(
                std::string id,
                std::filesystem::path file,
                Initializer initializer
            )
            {
                return create_tagged_input(
                    std::move(id),
                    std::move(file),
                    from_data(std::move(initializer))
                );
            }

            template <typename Initializer>
                requires(is_initializer<Initializer, std::string_view>)
            auto& create_tagged_input(
                std::string id,
                std::filesystem::path file,
                Initializer initializer
            )
            {
                using type = decltype(initializer(std::declval<std::string_view>()));
                auto ptr = std::make_unique<input::tagged::Frame<type>>(
                    std::move(id),
                    std::move(file),
                    std::move(initializer)
                );

                auto* raw_ptr = ptr.get();
                frames.emplace_back(std::move(ptr));
                return *raw_ptr;
            }

            template <typename Initializer>
                requires(!is_initializer<Initializer>)
            auto& create_unique_input(
                std::string id,
                std::filesystem::path file,
                Initializer initializer
            )
            {
                return create_unique_input(
                    std::move(id),
                    std::move(file),
                    from_data(std::move(initializer))
                );
            }

            template <typename Initializer>
                requires(is_initializer<Initializer>)
            auto& create_unique_input(
                std::string id,
                std::filesystem::path file,
                Initializer initializer
            )
            {
                using type = decltype(initializer());
                auto ptr = std::make_unique<input::unique::Frame<type>>(
                    std::move(id),
                    std::move(file),
                    std::move(initializer)
                );

                auto* raw_ptr = ptr.get();
                frames.emplace_back(std::move(ptr));
                return *raw_ptr;
            }

            template <typename T>
            auto& create_output(
                std::string id,
                std::filesystem::path file,
                test::type<T> /* type */ = {}
            )
            {
                auto ptr = std::make_unique<output::Frame<T>>(std::move(id), std::move(file));

                auto* raw_ptr = ptr.get();
                frames.emplace_back(std::move(ptr));
                return *raw_ptr;
            }

            void install(App& app, const std::filesystem::path& path) const
            {
                for (const auto& frame : frames)
                {
                    frame->install(app, path);
                }
            }

            std::vector<std::unique_ptr<IFrame>> frames;
        };

        struct TestBuilder
        {
            template <typename T>
            void add_test(std::string suite_id, std::string test_id, std::filesystem::path path)
            {
                tests.emplace_back(
                    [suite_id = std::move(suite_id),
                     test_id = std::move(test_id),
                     path = std::move(path)](App& app, const std::filesystem::path& relative_path)
                    {
                        for (const auto& dir :
                             std::filesystem::directory_iterator(relative_path / path))
                        {
                            if (dir.is_directory())
                            {
                                auto tag                             //
                                    = suite_id                       //
                                    + '.'                            //
                                    + test_id                        // NOLINT
                                    + '['                            //
                                    + dir.path().filename().string() //
                                    + ']';
                                app.install<T>(tag, type<typename T::base_t>());
                            }
                        }
                    }
                );
            }

            void install(App& app, const std::filesystem::path& path) const
            {
                for (const auto& t : tests)
                {
                    t(app, path);
                }
            }

            std::vector<std::function<void(App&, const std::filesystem::path&)>> tests;
        };
    }
}

template <typename... T>
struct std::tuple_size<nil::xit::test::InputData<T...>>
    : std::integral_constant<std::size_t, sizeof...(T)>
{
};

template <typename... T>
struct std::tuple_size<nil::xit::test::OutputData<T...>>
    : std::integral_constant<std::size_t, sizeof...(T)>
{
};

template <std::size_t I, typename... T>
    requires(I < sizeof...(T))
struct std::tuple_element<I, nil::xit::test::InputData<T...>>
{
    using type = decltype(*std::get<I>(std::declval<nil::xit::test::InputData<T...>>().data));
};

template <std::size_t I, typename... T>
    requires(I < sizeof...(T))
struct std::tuple_element<I, nil::xit::test::OutputData<T...>>
{
    using type = decltype(*std::get<I>(std::declval<nil::xit::test::OutputData<T...>>().data));
};
