
include(ExternalProject)

set(TARBALL_FILE "xit-0.4.30.tgz")
set(TARBALL_URL "https://registry.npmjs.org/@nil-/xit/-/${TARBALL_FILE}")
set(EXTRACT_DIR "${CMAKE_BINARY_DIR}/assets/ffi/py")
set(ASSETS_SRC_DIR "${EXTRACT_DIR}/assets")
set(ASSETS_DST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/nil_xit/assets")

ExternalProject_Add(
    ffi_xit_assets_download
    PREFIX ${CMAKE_BINARY_DIR}/ext_proj_add/ffi/py
    URL ${TARBALL_URL}
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/dl/ffi/py
    SOURCE_DIR ${EXTRACT_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SRC_DIR} ${ASSETS_DST_DIR}
    LOG_DOWNLOAD ON
    LOG_BUILD ON
)

setup_pypi("${CMAKE_SOURCE_DIR}/cmake/pyproject.toml.in")