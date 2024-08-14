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
set(SHADER_DIR ${RES_DIR}/shaders)
file(GLOB SHADERS ${SHADER_DIR}/*.vert ${SHADER_DIR}/*.frag ${SHADER_DIR}/*.comp ${SHADER_DIR}/*.geom ${SHADER_DIR}/*.tesc ${SHADER_DIR}/*.tese ${SHADER_DIR}/*.mesh ${SHADER_DIR}/*.task ${SHADER_DIR}/*.rgen ${SHADER_DIR}/*.rchit ${SHADER_DIR}/*.rmiss)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILENAME}.spv
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILENAME}.spv
        DEPENDS ${SHADER}
        COMMENT "GLSLC: Compiling ${FILENAME}")
list(APPEND SPV_SHADERS ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILENAME}.spv)
endForeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
add_executable(
    vulkanApp 
    ${SRC_DIR}/main.cpp 
    ${SRC_DIR}/vulkan_app.cpp
    ${SRC_DIR}/load_shader.cpp  
    ${SRC_DIR}/vertex_data.cpp
    ${SRC_DIR}/instance_creation.cpp
    ${SRC_DIR}/device_creation.cpp
    ${SRC_DIR}/buffer_creation.cpp
    ${SRC_DIR}/swapchain_creation.cpp
    ${SRC_DIR}/graphics_commands.cpp
    ${SRC_DIR}/sync_objects.cpp
    ${SRC_DIR}/command_buffer.cpp
    ${SRC_DIR}/uniform_buffers.cpp
    ${SRC_DIR}/timer.cpp
    ${SHADERS})
add_dependencies(vulkanApp shaders)

target_link_libraries(
    vulkanApp PRIVATE
    ${SDL2_LIBRARIES} 
    Vulkan::Vulkan)
    
include_directories(${SDL2_INCLUDE_DIRS})