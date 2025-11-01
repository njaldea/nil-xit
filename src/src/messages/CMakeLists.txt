project(xit-fbs)

find_package(flatbuffers CONFIG REQUIRED)
find_program(FLATC_EXECUTABLE flatc REQUIRED)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/message.fbs.h
    COMMAND
        ${FLATC_EXECUTABLE}
            --cpp
            --gen-object-api
            --schema
            -o ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/message.fbs
    COMMAND
        ${CMAKE_COMMAND}
            -E rename
            ${CMAKE_CURRENT_SOURCE_DIR}/message_generated.h
            ${CMAKE_CURRENT_SOURCE_DIR}/message.fbs.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/message.fbs
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating Flatbuffers code (cpp) from message.fbs"
)

add_library(${PROJECT_NAME} message.fbs.h)
set_property(TARGET ${PROJECT_NAME} PROPERTY LINKER_LANGUAGE CXX)
target_link_libraries(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:flatbuffers::flatbuffers>)
# set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "")

nil_install_targets(${PROJECT_NAME})