project(sandbox_basic)

add_executable(
    ${PROJECT_NAME}
    main.cpp
    add_frame.cpp
    add_frame.hpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE xit)