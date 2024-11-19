#include <nil/xit/structs.hpp>

#include <nil/service/http/server/create.hpp>

#include <fstream>
#include <iostream>

namespace nil::xit
{
    nil::service::H make_server(const HTTPServerOptions& options)
    {
        auto http_server = nil::service::http::server::create(
            {.port = options.port, .buffer = options.buffer_size}
        );

        on_get(
            http_server,
            [source_path = options.source_path](const auto& transaction)
            {
                auto route = get_route(transaction);
                if (route[0] == '/' && (route.size() == 1 || route[1] == '?'))
                {
                    set_content_type(transaction, "text/html");
                    const std::ifstream file(source_path / "assets/index.html", std::ios::binary);
                    send(transaction, file);
                }
                else
                {
                    const std::filesystem::path path = source_path / route.substr(1);
                    if (exists(path))
                    {
                        const std::ifstream file(path, std::ios::binary);
                        if (".js" == path.extension())
                        {
                            set_content_type(transaction, "application/javascript");
                            send(transaction, file);
                        }
                        else if (".png" == path.extension())
                        {
                            set_content_type(transaction, "image/png");
                            send(transaction, file);
                        }
                        else if (".svg" == path.extension())
                        {
                            set_content_type(transaction, "image/svg+xml");
                            send(transaction, file);
                        }
                    }
                }
            }
        );
        return http_server;
    }
}
