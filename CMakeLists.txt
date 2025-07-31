cmake_minimum_required(VERSION 3.14)

project("Renderer")

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS} include)

add_executable(${PROJECT_NAME} main.cpp src/vec3.cpp src/triangle.cpp src/renderedObj.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})