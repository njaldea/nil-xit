project(xit)

find_package(Protobuf CONFIG REQUIRED)

set(GENERATED_PROTOBUF_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen/xit/messages")

protobuf_generate_cpp(
    PROTO_SRC
    PROTO_HEADER
    src/message.proto
    PROTOC_OUT_DIR ${GENERATED_PROTOBUF_OUTPUT_DIR}
    OUTPUT_DIRECTORY_H ${GENERATED_PROTOBUF_OUTPUT_DIR}
)

add_library(${PROJECT_NAME}_proto ${PROTO_HEADER} ${PROTO_SRC})
target_link_libraries(${PROJECT_NAME}_proto PUBLIC protobuf::libprotobuf-lite)
set_target_properties(${PROJECT_NAME}_proto PROPERTIES CXX_CLANG_TIDY "")
target_include_directories(${PROJECT_NAME}_proto SYSTEM PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/gen)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME}_proto PRIVATE -Wno-sign-conversion)
endif()

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

# install_for_package(${PROJECT_NAME})