#include <nil/xit/structs.hpp>

#include <nil/service/http/server/create.hpp>
#include <nil/service/structs.hpp>

#include <fstream>
#include <string_view>

#ifndef NIL_XIT_ASSET_VERSION
static_assert(false, "requires NIL_XIT_ASSET_VERSION");
#endif

namespace
{
    constexpr std::string_view index_html = R"HTML(<!doctype html>
<html lang="en">
    <head>
        <meta charset="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <link rel="icon" href="https://unpkg.com/@nil-/xit@)HTML" NIL_XIT_ASSET_VERSION
                                            R"HTML(/assets/favicon.svg" />
        <meta property="og:image" content="https://unpkg.com/@nil-/xit@)HTML" NIL_XIT_ASSET_VERSION
                                            R"HTML(/assets/icon.png" />
        <meta property="og:title" content="nil/xit" />
        <meta property="og:description" content="C++ w/ svelte ui" />
        <title>nil/xit</title>
        <style>
            html,
            body {
                width: 100%;
                height: 100%;
                margin: 0px;
                padding: 0px;
            }
        </style>
        <script type="module" defer>
            import { create_component } from "https://unpkg.com/@nil-/xit@)HTML" NIL_XIT_ASSET_VERSION
                                            R"HTML(/assets/index.js";
            const params = new URLSearchParams(location.search);
            const ws_url = new URL("/ws", location.origin);
            ws_url.protocol = location.protocol === "https:" ? "wss:" : "ws:";
            const destroy = await create_component(document.getElementById("xit"), {
                host: ws_url.toString(),
                frame: params.get("frame") ?? "index",
                tag: params.get("tag"),
                cdn_url: "https://unpkg.com"
            });
        </script>
    </head>
    <body>
        <div id="xit" style="display: contents"></div>
    </body>
</html>
)HTML";

    bool is_index_route(std::string_view route)
    {
        return route == "/" || (route.size() > 1 && route[0] == '/' && route[1] == '?');
    }
}

namespace nil::xit
{
    void setup_server(service::IWebService& server, std::filesystem::path asset_path)
    {
        server.on_get(
            [asset_path = std::move(asset_path)](service::WebTransaction& transaction)
            {
                auto route = get_route(transaction);
                const auto is_index = route[0] == '/' && (route.size() == 1 || route[1] == '?');
                const auto file = is_index ? "index.html" : route.substr(1);

                if (file.ends_with(".html"))
                {
                    set_content_type(transaction, "text/html");
                }
                else if (file.ends_with(".svelte"))
                {
                    set_content_type(transaction, "text/plain");
                }
                else if (file.ends_with(".json"))
                {
                    set_content_type(transaction, "application/json");
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

                const auto full_path = asset_path / file;
                if (std::filesystem::exists(full_path))
                {
                    const std::ifstream f(full_path, std::ios::binary);
                    send(transaction, f);
                    return true;
                }
                return false;
            }
        );
    }

    void setup_svelte_server(service::IWebService& server)
    {
        server.on_get(
            [](service::WebTransaction& transaction)
            {
                const auto route = get_route(transaction);
                if (!is_index_route(route))
                {
                    return false;
                }

                set_content_type(transaction, "text/html");
                send(transaction, index_html);
                return true;
            }
        );
    }
}
