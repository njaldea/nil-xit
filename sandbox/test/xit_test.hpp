#pragma once

#include <nil/xit.hpp>

#include <nil/gate.hpp>
#include <nil/gate/bias/nil.hpp>

#include <string_view>
#include <unordered_map>
#include <utility>

namespace nil::xit::test
{
    namespace transparent
    {
        struct Hash
        {
            using is_transparent = void;

            std::size_t operator()(std::string_view s) const;
        };

        struct Equal
        {
            using is_transparent = void;

            bool operator()(std::string_view l, std::string_view r) const;
        };

        template <typename T>
        using hash_map = std::unordered_map<std::string, T, Hash, Equal>;
    }

    struct RerunTag
    {
        bool operator==(const RerunTag& /* o */) const;
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
            };

            namespace unique
            {
                template <typename T>
                struct Info final: input::Info<T>
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
                        if (info.input == nullptr)
                        {
                            info.input = gate->edge<T>();
                        }
                        return info.input;
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
                struct Info final: input::Info<T>
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
                        return info
                            .emplace(tag, typename Info<T>::Entry{std::nullopt, gate->edge<T>()})
                            .first->second.input;
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
        static constexpr auto* value = &S.value[0];
    };

    /**
     *  InputFrames/OutputFrames expect `Frame<StringLiteral, T>...` as arguments
     *  InputData/OutputData == argument of run
     *  Input/Output expect only `StringLiteral...` as arguments
     *      then expands to InputFrames/OutputFrames via Test's specialization
     *
     *  This works for compile time definitions. This is not applicable for runtime defined tests
     * (for example, in scripting languages)
     */

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

    template <typename T>
    auto from_data(T data)
    {
        struct Loader final
        {
            auto operator()(std::string_view /* tag */) const
            {
                return data;
            }

            auto operator()() const
            {
                return data;
            }

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

    class App
    {
    public:
        App(nil::service::S& service, std::string_view app_name);
        App(nil::service::HTTPService& service, std::string_view app_name);

        ~App() noexcept = default;
        App(App&&) = delete;
        App(const App&) = delete;
        App& operator=(App&&) = delete;
        App& operator=(const App&) = delete;

        const std::vector<std::string>& installed_tags() const;

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
                std::make_tuple(get_input<typename I::type>(I::value)...), // NOLINT
                std::make_tuple(get_output<typename O::type>(O::value)...) // NOLINT
            );
        }

        template <typename FromVS>
            requires requires(FromVS converter) {
                { converter(std::declval<std::vector<std::string>>()) };
            }
        void add_main(const std::filesystem::path& path, FromVS converter)
        {
            auto& f = add_unique_frame(xit, "demo", path);
            add_value(f, "tags", from_data(converter(installed_tags())));
            add_value(f, "outputs", from_data(converter({"view_frame"})));
            add_value(f, "inputs", from_data(converter({"slider_frame", "input_frame"})));
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
        nil::xit::C xit;
        nil::gate::Core gate;

        transparent::hash_map<std::unique_ptr<frame::IInfo>> input_frames;
        transparent::hash_map<std::unique_ptr<frame::IInfo>> output_frames;

        std::vector<std::string> tags;

        template <typename Callable, typename... Inputs>
        void add_node(
            std::string_view tag,
            Callable callable,
            std::tuple<Inputs...> inputs,
            std::tuple<> /* outputs */
        )
        {
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
    };
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
