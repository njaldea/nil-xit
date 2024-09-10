find_package(Protobuf CONFIG REQUIRED)

protobuf_generate_cpp(
    PROTO_SRC
    PROTO_HEADER
    message.proto
    PROTOC_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_DIRECTORY_H ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(${PROJECT_NAME}_proto ${PROTO_HEADER} ${PROTO_SRC})
target_link_libraries(${PROJECT_NAME}_proto PUBLIC protobuf::libprotobuf-lite)
set_target_properties(${PROJECT_NAME}_proto PROPERTIES CXX_CLANG_TIDY "")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME}_proto PRIVATE -Wno-sign-conversion)
endif()

install(TARGETS ${PROJECT_NAME}_proto EXPORT ${CMAKE_PROJECT_NAME}-${PROJECT_NAME}_proto-targets)
install(
    EXPORT ${CMAKE_PROJECT_NAME}-${PROJECT_NAME}_proto-targets
    NAMESPACE ${CMAKE_PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}
)