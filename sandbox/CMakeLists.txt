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

find_package(nil-service CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE xit nil::service)

if(ENABLE_C_API)
    find_package(nil-service CONFIG REQUIRED c-api)
    add_executable(${PROJECT_NAME}-c main.c)
    target_link_libraries(${PROJECT_NAME}-c PRIVATE xit-c-api nil::service-c-api)
endif()
