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

        on_ready(http_server, []() { std::cout << "http://localhost:1101" << std::endl; });

        on_get(
            http_server,
            [source_path = options.source_path](const auto& transaction)
            {
                auto route = get_route(transaction);
                if ("/" == route || (route[0] == '/' && route[1] == '?'))
                {
                    std::ifstream file(
                        source_path / "node_modules/@nil-/xit/assets/index.html",
                        std::ios::binary
                    );
                    send(transaction, "text/html", file);
                }
                else
                {
                    const std::filesystem::path path
                        = source_path / "node_modules/@nil-/xit" / route.substr(1);
                    if (exists(path))
                    {
                        std::ifstream file(path, std::ios::binary);
                        if (".js" == path.extension())
                        {
                            send(transaction, "application/javascript", file);
                            return;
                        }
                        if (".png" == path.extension())
                        {
                            send(transaction, "image/png", file);
                            return;
                        }
                        if (".svg" == path.extension())
                        {
                            send(transaction, "image/svg+xml", file);
                            return;
                        }
                    }
                }
            }
        );
        return http_server;
    }
}
