#include "xit_test.hpp"

#include <nil/gate/runners/NonBlocking.hpp>

namespace nil::xit::test
{
    namespace transparent
    {
        std::size_t Hash::operator()(std::string_view s) const
        {
            return std::hash<std::string_view>()(s);
        }

        bool Equal::operator()(std::string_view l, std::string_view r) const
        {
            return l == r;
        }
    }

    bool RerunTag::operator==(const RerunTag& /* o */) const
    {
        return false;
    }

    App::App(nil::service::S& service, std::string_view app_name)
        : xit(nil::xit::make_core(service))
    {
        gate.set_runner<nil::gate::runners::NonBlocking>();
        on_ready(service, [this]() { gate.commit(); });
        set_cache_directory(xit, std::filesystem::temp_directory_path() / app_name);
    }

    App::App(nil::service::HTTPService& service, std::string_view app_name)
        : xit(nil::xit::make_core(service))
    {
        gate.set_runner<nil::gate::runners::NonBlocking>();
        on_ready(service, [this]() { gate.commit(); });
        set_cache_directory(xit, std::filesystem::temp_directory_path() / app_name);
    }

    const std::vector<std::string>& App::installed_tags() const
    {
        return tags;
    }

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
}
