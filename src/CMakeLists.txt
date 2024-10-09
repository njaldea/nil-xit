project(xit)

add_subdirectory(src/proto)

find_package(nil-service CONFIG REQUIRED)

set(
    SOURCES
        src/codec.hpp
        src/structs.hpp
        src/utils.hpp
        src/utils.cpp
        src/create_core.cpp
        src/add_frame.cpp

        src/unique/structs.hpp
        src/unique/bind.cpp
        src/unique/listen.cpp
        src/unique/post.cpp
        src/unique/utils.cpp
        src/unique/utils.hpp

        src/tagged/structs.hpp
        src/tagged/bind.cpp
        src/tagged/listen.cpp
        src/tagged/post.cpp
        src/tagged/utils.cpp
        src/tagged/utils.hpp
)

set(
    HEADERS
        publish/nil/xit.hpp
        publish/nil/xit/structs.hpp
        publish/nil/xit/add_frame.hpp
        publish/nil/xit/unique/bind.hpp
        publish/nil/xit/unique/listen.hpp
        publish/nil/xit/unique/post.hpp
        publish/nil/xit/tagged/bind.hpp
        publish/nil/xit/tagged/listen.hpp
        publish/nil/xit/tagged/post.hpp
)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE xit-proto)
target_link_libraries(${PROJECT_NAME} PUBLIC nil::service)

target_include_directories(
    ${PROJECT_NAME}
    SYSTEM PRIVATE
    $<TARGET_PROPERTY:Boost::boost,INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_definitions(${PROJECT_NAME} PRIVATE BOOST_ASIO_NO_DEPRECATED)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/publish>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

nil_install_headers()
nil_install_targets(${PROJECT_NAME})