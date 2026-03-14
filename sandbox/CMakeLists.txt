project(sandbox)

set(ENABLE_SANDBOX  OFF CACHE BOOL "[0 | OFF - 1 | ON]: build sandbox?")
if(NOT ENABLE_SANDBOX)
    return()
endif()

add_executable(
    ${PROJECT_NAME}
    main.cpp
    add_frame.cpp
    add_frame.hpp
)

find_package(nil-service CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE xit nil::service)

include(ExternalProject)

set(TARBALL_FILE "xit-0.4.19.tgz")
set(TARBALL_URL "https://registry.npmjs.org/@nil-/xit/-/${TARBALL_FILE}")
set(TARBALL_PATH "${CMAKE_BINARY_DIR}/${TARBALL_FILE}")
set(EXTRACT_DIR "${CMAKE_BINARY_DIR}/assets/xit")

ExternalProject_Add(
    xit_assets_download
    PREFIX ${CMAKE_BINARY_DIR}/xit_dl
    URL ${TARBALL_URL}
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
    SOURCE_DIR ${CMAKE_BINARY_DIR}/assets/xit
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_BUILD ON
)