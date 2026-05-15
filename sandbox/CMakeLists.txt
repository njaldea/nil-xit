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

if(ENABLE_C_API)
    find_package(nil-service CONFIG REQUIRED c-api)
    add_executable(${PROJECT_NAME}-c main.c)
    target_link_libraries(${PROJECT_NAME}-c PRIVATE xit-c-api nil::service-c-api)
endif()

include(ExternalProject)

set(TARBALL_FILE "xit-0.4.30.tgz")
set(TARBALL_URL "https://registry.npmjs.org/@nil-/xit/-/${TARBALL_FILE}")
set(TARBALL_PATH "${CMAKE_BINARY_DIR}/${TARBALL_FILE}")
set(EXTRACT_DIR "${CMAKE_BINARY_DIR}/assets/sandbox")

ExternalProject_Add(
    sandbox_xit_assets_download
    PREFIX ${CMAKE_BINARY_DIR}/ext_proj_add/sandbox
    URL ${TARBALL_URL}
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/dl/sandbox
    SOURCE_DIR ${EXTRACT_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_BUILD ON
)