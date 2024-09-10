project(xit)

add_subdirectory(src/proto)

find_package(nil-service CONFIG REQUIRED)

set(
    SOURCES
        src/codec.hpp
        src/structs.hpp
        src/methods/make_core.cpp
        src/methods/add_frame.cpp
        src/methods/bind.cpp
        src/methods/get.cpp
        src/methods/set.cpp
        src/methods/post.cpp
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
        publish/nil/xit/methods/make_core.hpp
)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_proto)
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

install_for_package(${PROJECT_NAME})