project(xit)

add_subdirectory(src/messages)

find_package(nil-service CONFIG REQUIRED)

set(
    SOURCES
        src/codec.hpp
        src/structs.hpp
        src/utils.hpp
        src/create_core.cpp
        src/add_frame.cpp
        src/make_server.cpp

        src/unique/structs.hpp
        src/unique/add_value.cpp
        src/unique/add_signal.cpp
        src/unique/on_load.cpp
        src/unique/on_sub.cpp
        src/unique/post.cpp
        src/unique/utils.cpp
        src/unique/utils.hpp

        src/tagged/structs.hpp
        src/tagged/add_value.cpp
        src/tagged/add_signal.cpp
        src/tagged/on_load.cpp
        src/tagged/on_sub.cpp
        src/tagged/post.cpp
        src/tagged/utils.cpp
        src/tagged/utils.hpp
)

set(
    HEADERS
        publish/nil/xit.hpp
        publish/nil/xit/structs.hpp
        publish/nil/xit/add_frame.hpp
        publish/nil/xit/buffer_type.hpp
        publish/nil/xit/unique/add_value.hpp
        publish/nil/xit/unique/add_signal.hpp
        publish/nil/xit/unique/on_load.hpp
        publish/nil/xit/unique/on_sub.hpp
        publish/nil/xit/unique/post.hpp
        publish/nil/xit/unique/structs.hpp
        publish/nil/xit/tagged/add_value.hpp
        publish/nil/xit/tagged/add_signal.hpp
        publish/nil/xit/tagged/on_load.hpp
        publish/nil/xit/tagged/on_sub.hpp
        publish/nil/xit/tagged/post.hpp
        publish/nil/xit/tagged/structs.hpp
)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE xit-fbs)
target_link_libraries(${PROJECT_NAME} PUBLIC nil::service)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

nil_install_headers()
nil_install_targets(${PROJECT_NAME})