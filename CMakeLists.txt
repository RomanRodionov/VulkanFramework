cmake_minimum_required(VERSION 3.25.0)
project(vulkanApp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(lib/glfw-3.4)

SET(GLM_TEST_ENABLE OFF CACHE BOOL "GLM Build unit tests")
add_subdirectory(lib/glm EXCLUDE_FROM_ALL)

add_executable(vulkanApp main.cpp)

target_link_libraries(vulkanApp glfw Vulkan::Vulkan)