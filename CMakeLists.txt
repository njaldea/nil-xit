cmake_minimum_required(VERSION 3.27.1)

project(
    nil-xit
    VERSION 0.3.6
    LANGUAGES CXX
    DESCRIPTION "C++ bridge to frontend"
)

find_package(nil-service CONFIG REQUIRED)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "nil-service-dev (>= ${nil-service_VERSION})")

find_package(nil-cmake CONFIG REQUIRED)
install_config("cmake/config.cmake.in")

add_subdirectory(src)
add_subdirectory(sandbox)
