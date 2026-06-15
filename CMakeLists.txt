cmake_minimum_required(VERSION 3.27.1)

project(
    nil-xit
    VERSION 0.3.23
    LANGUAGES CXX
    DESCRIPTION "C++ bridge to frontend"
)

set(NIL_XIT_ASSET_VERSION "0.4.40" CACHE STRING "Version of @nil-/xit to use")

find_package(nil-service CONFIG REQUIRED)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "nil-service-dev (>= ${nil-service_VERSION})")

find_package(nil-cmake CONFIG REQUIRED)
install_config("cmake/config.cmake.in")

set(ENABLE_C_API OFF CACHE BOOL "[0 | OFF - 1 | ON]: build c api?")

add_subdirectory(src)
add_subdirectory(sandbox)
