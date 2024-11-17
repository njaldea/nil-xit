

set(ENABLE_SANDBOX  OFF CACHE BOOL "[0 | OFF - 1 | ON]: build sandbox?")
if(NOT ENABLE_SANDBOX)
    return()
endif()

find_package(nlohmann_json CONFIG REQUIRED)
find_package(nil-gate CONFIG REQUIRED)

project(xit_nil_gtest)
add_library(
    ${PROJECT_NAME}
    xit_gtest.cpp
    xit_gtest.hpp
    xit_test.cpp
    xit_test.hpp
)
target_link_libraries(${PROJECT_NAME} PUBLIC xit)
target_link_libraries(${PROJECT_NAME} PUBLIC nil::gate)

project(sandbox_test)

add_executable(
    ${PROJECT_NAME}
    main.cpp
    userland_utils.cpp
    userland_utils.hpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE xit_nil_gtest)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)