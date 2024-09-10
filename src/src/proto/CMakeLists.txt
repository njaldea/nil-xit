project(${PROJECT_NAME}-proto)

find_package(Protobuf CONFIG REQUIRED)

protobuf_generate_cpp(
    PROTO_SRC
    PROTO_HEADER
    message.proto
    PROTOC_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_DIRECTORY_H ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(${PROJECT_NAME} ${PROTO_HEADER} ${PROTO_SRC})
target_link_libraries(${PROJECT_NAME} PUBLIC protobuf::libprotobuf-lite)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-sign-conversion)
endif()

nil_install_targets(${PROJECT_NAME})