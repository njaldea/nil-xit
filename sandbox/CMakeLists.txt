project(sandbox)

set(ENABLE_SANDBOX  OFF CACHE BOOL "[0 | OFF - 1 | ON]: build sandbox?")
if(NOT ENABLE_SANDBOX)
    return()
endif()

add_executable(
    ${PROJECT_NAME}
    sandbox.cpp
    add_frame.cpp
    add_frame.hpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE xit)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(sandbox_test sandbox_test.cpp)
target_link_libraries(sandbox_test PRIVATE xit)
target_link_libraries(sandbox_test PRIVATE nlohmann_json::nlohmann_json)