cmake_minimum_required(VERSION 3.25.0)
project(vulkanApp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)

find_package(SDL2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

SET(GLM_TEST_ENABLE OFF CACHE BOOL "GLM Build unit tests")
add_subdirectory(lib/glm EXCLUDE_FROM_ALL)

add_executable(vulkanApp main.cpp)

target_link_libraries(
    vulkanApp PRIVATE
    ${SDL2_LIBRARIES} 
    Vulkan::Vulkan)

    
include_directories(${SDL2_INCLUDE_DIRS})