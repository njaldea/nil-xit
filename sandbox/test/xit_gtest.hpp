#pragma once

#include "xit_test.hpp"

#include <filesystem>

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
}

// NOLINTNEXTLINE
#define XIT_CONCAT_D(A, B) A_##B
// NOLINTNEXTLINE
#define XIT_CONCAT(A, B) XIT_CONCAT_D(A, B)
// NOLINTNEXTLINE
#define XIT_TEST(SUITE, CASE, DIRECTORY)                                                           \
    struct XIT_CONCAT(XTEST, SUITE_##CASE)                                                         \
        : SUITE                                                                                    \
    {                                                                                              \
        void run(const inputs_t& xit_inputs, outputs_t& xit_outputs) override;                     \
    };                                                                                             \
    const auto xit_test_##__COUNTER__ = []()                                                       \
    {                                                                                              \
        nil::xit::gtest::get_instance()                                                            \
            .test_builder.add_test<XIT_CONCAT(XTEST, SUITE_##CASE)>(#SUITE, #CASE, DIRECTORY);     \
        return 0;                                                                                  \
    }();                                                                                           \
    void XIT_CONCAT(XTEST, SUITE_##CASE)::run(const inputs_t& xit_inputs, outputs_t& xit_outputs)

// NOLINTNEXTLINE
#define XIT_FRAME_TAGGED_INPUT(ID, PATH, INITIALIZER)                                              \
    const auto& XIT_CONCAT(xit_test, __COUNTER__)                                                  \
        = nil::xit::gtest::get_instance().frame_builder.create_tagged_input(ID, PATH, INITIALIZER)
// NOLINTNEXTLINE
#define XIT_FRAME_UNIQUE_INPUT(ID, PATH, INITIALIZER)                                              \
    const auto& XIT_CONCAT(xit_test, __COUNTER__)                                                  \
        = nil::xit::gtest::get_instance().frame_builder.create_unique_input(ID, PATH, INITIALIZER)
// NOLINTNEXTLINE
#define XIT_FRAME_OUTPUT(ID, PATH, TYPE)                                                           \
    const auto& XIT_CONCAT(xit_test, __COUNTER__)                                                  \
        = nil::xit::gtest::get_instance()                                                          \
              .frame_builder.create_output(ID, PATH, nil::xit::test::type<TYPE>())

// NOLINTNEXTLINE
#define XIT_USE_DIRECTORY(PATH)                                                                    \
    const auto xit_rel_path_##__COUNTER__ = []()                                                   \
    {                                                                                              \
        nil::xit::gtest::get_instance().relative_path = PATH;                                      \
        return 0;                                                                                  \
    }();
