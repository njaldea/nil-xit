# nil/xit

This is a small project designed to bridge the gap between C++ and any UI frontend to create GUI applications.

## Motivation

Popular GUI frameworks like ImGui and Qt require developers to learn their specific APIs, rendering models, and threading models.

This project provides an alternative approach to building GUI applications, minimizing direct interaction with the GUI thread.

## Requirements

This library is developed in a repo with `vcpkg`.

1. To consume the library, add `nil` registry to your `vcpkg-configuration.json`

```json
{
    "$schema": "https://raw.githubusercontent.com/microsoft/vcpkg-tool/main/docs/vcpkg-configuration.schema.json",
    "default-registry": {
        "kind": "git",
        "repository": "https://github.com/Microsoft/vcpkg",
        "baseline": "3508985146f1b1d248c67ead13f8f54be5b4f5da"
    },
    "registries": [
        {
            "kind": "git",
            "repository": "https://github.com/njaldea/nil-vcpkg-ports",
            "baseline": "cd90893b3a88f7dadda67c68b7a8050c7651920e",
            "packages": ["nil-xit", "nil-xalt", "nil-service"]
        }
    ]
}
```

2. Create your target binary and link to `nil::xit`.

```
project(YOUR_PROJECT)

find_package(nil-xit CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::xit)
```

3. Implement your application

The example below serves the GUI through http server.

3.A If you want to serve your own files

```cpp
#include <nil/service.hpp>
#include <nil/xit.hpp>

int main()
{
    auto server = nil::service::http::server::create({
        .host = "127.0.0.1",
        .port = 1101,
        .buffer = 1024ul * 1024ul * 100ul
    });

    // if using `nil-/xit`, run npm install with the package.json below
    // and point to the library like below.
    // this is going to be simplified once #embed is available
    nil::xit::setup_server(server, {"node_modules/@nil-/xit/assets"});
    auto ws = use_ws(server, "/ws");
    auto core = nil::xit::make_core(ws);

    start(server); // blocking call
    return 0;
}
```

While the source files are not yet embedded in the library (`#embed`), 
a sibling library `nil-/xit` in npmjs is developed so we can write the UI in svelte.

```json
{
  "dependencies": {
    "@nil-/xit": "^0.2.8",
    "svelte": "^5.33.4"
  }
}
```

### If you want to reuse files served through vercel

```cpp
#include <nil/service.hpp>
#include <nil/xit.hpp>

int main()
{
    auto server = nil::service::ws::server::create({
        .host = "127.0.0.1",
        .port = 1101,
        .route = "/ws",
        .buffer = 1024ul * 1024ul * 100ul
    });

    auto core = nil::xit::make_core(server);

    start(server); // blocking call
    return 0;
}
```

Then visit: https://xit-ui.vercel.app/view/ws:/127.0.0.1:1101/ws

## Dependencies

 -  boost (asio and beast)
 -  flatbuffers
 -  [nil/service](https://github.com/njaldea/nil-service/blob/master/README.md)
 -  [nil/xalt](https://github.com/njaldea/nil-xalt/blob/master/README.md)

## [How It Works](./doc/01-How-It-Works.md)

## [Frames](./doc/02-Frames.md)

## [Supported Types](./doc/03-Supported-Types.md)

## [Number Issues](./doc/04-Number-Issues.md)