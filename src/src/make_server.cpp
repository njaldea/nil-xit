#include <nil/xit/structs.hpp>

#include <nil/service/http/server/create.hpp>
#include <nil/service/structs.hpp>

#include <fstream>

namespace nil::xit
{
    void setup_server(service::IWebService& server, std::vector<std::filesystem::path> asset_paths)
    {
        server.on_get(
            [asset_paths = std::move(asset_paths)](service::WebTransaction& transaction)
            {
                auto route = get_route(transaction);
                const auto is_index = route[0] == '/' && (route.size() == 1 || route[1] == '?');
                const auto file = is_index ? "index.html" : route.substr(1);

                if (file.ends_with(".html"))
                {
                    set_content_type(transaction, "text/html");
                }
                else if (file.ends_with(".js"))
                {
                    set_content_type(transaction, "application/javascript");
                }
                else if (file.ends_with(".png"))
                {
                    set_content_type(transaction, "image/png");
                }
                else if (file.ends_with(".svg"))
                {
                    set_content_type(transaction, "image/svg+xml");
                }
                else
                {
                    return false;
                }

                for (const auto& asset_path : asset_paths) // NOLINT(readability-use-anyofallof)
                {
                    const auto full_path = asset_path / file;
                    if (std::filesystem::exists(full_path))
                    {
                        const std::ifstream f(full_path, std::ios::binary);
                        send(transaction, f);
                        return true;
                    }
                }
                return false;
            }
        );
    }
}
