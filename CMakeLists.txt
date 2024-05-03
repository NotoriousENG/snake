cmake_minimum_required(VERSION 3.15)
project(snake)

find_package(raylib 3.0 REQUIRED) # Requires at least version 3.0

set(CMAKE_C_STANDARD 11) # Requires C11 standard

add_executable(${PROJECT_NAME} "./src/game.c" "./src/entity.c" "./src/tail.c" "./src/snake.c" "./src/input.c" "./src/ui.c")

target_include_directories(${PROJECT_NAME} PRIVATE "./include")

target_link_libraries(${PROJECT_NAME} raylib)

# add math lib
find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(${PROJECT_NAME} ${MATH_LIBRARY})
endif()

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()