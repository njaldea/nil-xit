project(sandbox_test)

set(ENABLE_SANDBOX  OFF CACHE BOOL "[0 | OFF - 1 | ON]: build sandbox?")
if(NOT ENABLE_SANDBOX)
    return()
endif()

find_package(nlohmann_json CONFIG REQUIRED)
find_package(nil-gate CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE xit)
target_link_libraries(${PROJECT_NAME} PRIVATE nil::gate)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)