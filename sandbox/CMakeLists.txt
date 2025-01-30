project(sandbox)

set(ENABLE_SANDBOX  OFF CACHE BOOL "[0 | OFF - 1 | ON]: build sandbox?")
if(NOT ENABLE_SANDBOX)
    return()
endif()

add_executable(
    ${PROJECT_NAME}
    main.cpp
    add_frame.cpp
    add_frame.hpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE xit)

find_package(flatbuffers CONFIG REQUIRED)
add_executable(${PROJECT_NAME}_fbs fbs.cpp)
target_include_directories(${PROJECT_NAME}_fbs PUBLIC ${CMAKE_SOURCE_DIR}/src/src/messages)
target_link_libraries(${PROJECT_NAME}_fbs PUBLIC flatbuffers::flatbuffers)