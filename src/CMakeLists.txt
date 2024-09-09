project(xit)

add_subdirectory(src/proto)

find_package(nil COMPONENTS service CONFIG REQUIRED)

set(
    SOURCES
        src/xit.cpp
        src/codec.hpp
)

set(
    HEADERS
        publish/nil/xit.hpp
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