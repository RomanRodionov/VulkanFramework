cmake_minimum_required(VERSION 3.25.0)
project(vulkanApp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

SET(GLM_TEST_ENABLE OFF CACHE BOOL "GLM Build unit tests")
add_subdirectory(extern/glm EXCLUDE_FROM_ALL)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(RES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(SHADERS_SRC ${SRC_DIR}/shaders)
file(GLOB SHADERS ${SHADERS_SRC}/*.vert ${SHADERS_SRC}/*.frag ${SHADERS_SRC}/*.comp ${SHADERS_SRC}/*.geom ${SHADERS_SRC}/*.tesc ${SHADERS_SRC}/*.tese ${SHADERS_SRC}/*.mesh ${SHADERS_SRC}/*.task ${SHADERS_SRC}/*.rgen ${SHADERS_SRC}/*.rchit ${SHADERS_SRC}/*.rmiss)

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/extern CACHE PATH "Missing description." FORCE)
FetchContent_Declare(assimp
GIT_REPOSITORY https://github.com/assimp/assimp.git
GIT_TAG master)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF)
FetchContent_MakeAvailable(assimp)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME)
    add_custom_command(OUTPUT ${RES_DIR}/shaders/${FILENAME}.spv
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${RES_DIR}/shaders/${FILENAME}.spv
        DEPENDS ${SHADER}
        COMMENT "GLSLC: Compiling ${FILENAME}")
    list(APPEND SPV_SHADERS ${RES_DIR}/shaders/${FILENAME}.spv)
endForeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

set(VULKAN_APP_SRC     
    ${SRC_DIR}/vulkan_app/vulkan_app.cpp
    ${SRC_DIR}/vulkan_app/load_shader.cpp  
    ${SRC_DIR}/vulkan_app/vertex_data.cpp
    ${SRC_DIR}/vulkan_app/instance_creation.cpp
    ${SRC_DIR}/vulkan_app/device_creation.cpp
    ${SRC_DIR}/vulkan_app/buffer_creation.cpp
    ${SRC_DIR}/vulkan_app/swapchain_creation.cpp
    ${SRC_DIR}/vulkan_app/graphics_commands.cpp
    ${SRC_DIR}/vulkan_app/sync_objects.cpp
    ${SRC_DIR}/vulkan_app/command_buffer.cpp
    ${SRC_DIR}/vulkan_app/uniform_buffers.cpp
    ${SRC_DIR}/vulkan_app/timer.cpp
    ${SRC_DIR}/vulkan_app/stb_implementation.cpp
    ${SRC_DIR}/vulkan_app/texture.cpp
    ${SRC_DIR}/vulkan_app/depth_buffering.cpp
    ${SRC_DIR}/vulkan_app/load_model.cpp)

add_executable(
    vulkanApp 
    ${SRC_DIR}/main.cpp 
    ${VULKAN_APP_SRC}
    ${SHADERS})
add_dependencies(vulkanApp shaders)

include_directories(extern/stb)
include_directories(${SDL2_INCLUDE_DIRS})
target_include_directories(vulkanApp PRIVATE extern/assimp-src/include)


target_link_libraries(
    vulkanApp
    ${SDL2_LIBRARIES} 
    Vulkan::Vulkan
    assimp)