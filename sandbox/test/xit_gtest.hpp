#pragma once

#include "xit_test.hpp"

#include <filesystem>
#include <fstream>

namespace nil::xit::gtest
{
    struct Instances
    {
        std::filesystem::path relative_path;
        nil::xit::test::builders::FrameBuilder frame_builder;
        nil::xit::test::builders::TestBuilder test_builder;
    };

    inline Instances& get_instance()
    {
        static auto instance = Instances{std::filesystem::current_path(), {}, {}};
        return instance;
    }

    template <typename Reader>
        requires requires(Reader reader) {
            { reader(std::declval<std::istream&>()) };
        }
    auto from_file(std::filesystem::path source_path, std::string file_name, Reader reader)
    {
        struct Loader final
        {
        public:
            Loader(
                std::filesystem::path init_source_path,
                std::string init_file_name,
                Reader init_reader
            )
                : source_path(std::move(init_source_path))
                , file_name(std::move(init_file_name))
                , reader(std::move(init_reader))
            {
            }

            ~Loader() noexcept = default;
            Loader(const Loader&) = default;
            Loader(Loader&&) = default;
            Loader& operator=(const Loader&) = default;
            Loader& operator=(Loader&&) = default;

            auto operator()(std::string_view tag) const
            {
                const auto i1 = tag.find_last_of('[') + 1;
                const auto i2 = tag.find_last_of(']');
                return load(
                    get_instance().relative_path / source_path / tag.substr(i1, i2 - i1) / file_name
                );
            }

            auto operator()() const
            {
                return load(get_instance().relative_path / source_path / file_name);
            }

        private:
            std::filesystem::path source_path;
            std::string file_name;
            Reader reader;

            auto load(const std::filesystem::path& path) const
            {
                if (!std::filesystem::exists(path))
                {
                    throw std::runtime_error("not found: " + path.string());
                }
                std::ifstream file(path, std::ios::binary);
                return reader(file);
            }
        };

        return Loader{std::move(source_path), std::move(file_name), std::move(reader)};
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define XIT_WRAP(A) A
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define XIT_CONCAT_D(A, B) A##B
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define XIT_CONCAT(A, B) XIT_CONCAT_D(A, B)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define XIT_TEST(SUITE, CASE, DIRECTORY)                                                           \
    struct xit_test_##SUITE##_##CASE: XIT_WRAP(SUITE)                                              \
    {                                                                                              \
        void run(const inputs_t& xit_inputs, outputs_t& xit_outputs) override;                     \
    };                                                                                             \
    const auto v_xit_test_##SUITE##_##CASE = []()                                                  \
    {                                                                                              \
        nil::xit::gtest::get_instance()                                                            \
            .test_builder.add_test<xit_test_##SUITE##_##CASE>(#SUITE, #CASE, DIRECTORY);           \
        return 0;                                                                                  \
    }();                                                                                           \
    void xit_test_##SUITE##_##CASE::run(const inputs_t& xit_inputs, outputs_t& xit_outputs)

// NOLINTNEXTLINE
#define XIT_FRAME_TAGGED_INPUT(ID, PATH, INITIALIZER)                                              \
    template <>                                                                                    \
    struct nil::xit::test::Frame<ID>                                                               \
        : nil::xit::test::Frame<                                                                   \
              ID,                                                                                  \
              std::remove_cvref_t<                                                                 \
                  decltype(nil::xit::gtest::get_instance()                                         \
                               .frame_builder.create_tagged_input(ID, PATH, INITIALIZER))>::type>  \
    {                                                                                              \
    };                                                                                             \
    const auto& XIT_CONCAT(xit_test_frame_, __COUNTER__)                                           \
        = nil::xit::gtest::get_instance().frame_builder.create_tagged_input(ID, PATH, INITIALIZER)

// NOLINTNEXTLINE
#define XIT_FRAME_UNIQUE_INPUT(ID, PATH, INITIALIZER)                                              \
    template <>                                                                                    \
    struct nil::xit::test::Frame<ID>                                                               \
        : nil::xit::test::Frame<                                                                   \
              ID,                                                                                  \
              std::remove_cvref_t<                                                                 \
                  decltype(nil::xit::gtest::get_instance()                                         \
                               .frame_builder.create_unique_input(ID, PATH, INITIALIZER))>::type>  \
    {                                                                                              \
    };                                                                                             \
    const auto& XIT_CONCAT(xit_test_frame_, __COUNTER__)                                           \
        = nil::xit::gtest::get_instance().frame_builder.create_unique_input(ID, PATH, INITIALIZER)

// NOLINTNEXTLINE
#define XIT_FRAME_OUTPUT(ID, PATH, TYPE)                                                           \
    template <>                                                                                    \
    struct nil::xit::test::Frame<ID>: nil::xit::test::Frame<ID, TYPE>                              \
    {                                                                                              \
    };                                                                                             \
    const auto& XIT_CONCAT(xit_test_frame_, __COUNTER__)                                           \
        = nil::xit::gtest::get_instance().frame_builder.create_output<TYPE>(ID, PATH)

// NOLINTNEXTLINE
#define XIT_USE_DIRECTORY(PATH)                                                                    \
    const auto v_xit_rel_path = []()                                                               \
    {                                                                                              \
        nil::xit::gtest::get_instance().relative_path = PATH;                                      \
        return 0;                                                                                  \
    }();
