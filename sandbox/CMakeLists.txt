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
