#include "vulkan_app.hpp"

void VulkanApp::beginTimer()
{
    startTime = std::chrono::high_resolution_clock::now();
}

float VulkanApp::getTime()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    return time;
}