#include "xit_gtest.hpp"

namespace nil::xit::gtest
{
    namespace builders
    {
        void MainBuilder::install(App& app, const std::filesystem::path& path) const
        {
            if (frame)
            {
                frame->install(app, path);
            }
        }

        void FrameBuilder::install(App& app, const std::filesystem::path& path) const
        {
            for (const auto& frame : frames)
            {
                frame->install(app, path);
            }
        }

        void TestBuilder::install(App& app, const std::filesystem::path& path) const
        {
            for (const auto& t : tests)
            {
                t(app, path);
            }
        }
    }

    inline Instances& get_instance()
    {
        static auto instance = Instances{};
        return instance;
    }

    int main(int argc, const char** argv)
    {
        // TODO: headless option. use nil/clix
        (void)argc;
        (void)argv;
        auto& instance = nil::xit::gtest::get_instance();

        const auto http_server = nil::xit::make_server({
            .source_path = instance.paths.server,
            .port = 1101,
            .buffer_size = 1024ul * 1024ul * 100ul //
        });

        nil::xit::test::App app(http_server, "nil-xit-gtest");
        instance.frame_builder.install(app, instance.paths.ui);
        instance.test_builder.install(app, instance.paths.test);
        instance.main_builder.install(app, instance.paths.main_ui);

        start(http_server);
        return 0;
    }
}

__attribute__((weak)) int main(int argc, const char** argv)
{
    return nil::xit::gtest::main(argc, argv);
}
