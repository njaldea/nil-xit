project(xit)

add_subdirectory(src/proto)

find_package(nil-service CONFIG REQUIRED)

set(
    SOURCES
        src/codec.hpp
        src/structs.hpp
        src/methods/create_core.cpp
        src/methods/add_frame.cpp
        src/methods/bind.cpp
        src/methods/get.cpp
        src/methods/set.cpp
        src/methods/post.cpp
        src/methods/listen.cpp
        src/methods/impl_set.cpp
        src/methods/impl_set.hpp
)

set(
    HEADERS
        publish/nil/xit.hpp
        publish/nil/xit/structs.hpp
        publish/nil/xit/methods/add_frame.hpp
        publish/nil/xit/methods/bind.hpp
        publish/nil/xit/methods/get.hpp
        publish/nil/xit/methods/set.hpp
        publish/nil/xit/methods/post.hpp
        publish/nil/xit/methods/listen.hpp
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