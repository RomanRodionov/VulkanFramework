#include "vulkan_app.hpp"

void VulkanApp::initWindow() 
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Vulkan_LoadLibrary(nullptr);
    window = SDL_CreateWindow("VulkanApp", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
}

void VulkanApp::initVulkan() 
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createCommandPools();
    createDepthResources();
    createFramebuffers();
    createTextureImage();
    createTextureImageView();
    createTextureSampler();
    loadModel();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
}

void VulkanApp::mainLoop() 
{
    beginTimer();

    bool running = true;
    while(running) 
    {
        SDL_Event windowEvent;
        while(SDL_PollEvent(&windowEvent))
        {
            if(windowEvent.type == SDL_QUIT) 
            {
                running = false;
                break;
            }
            else if (windowEvent.type == SDL_WINDOWEVENT)
            {
                if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    framebufferResized = true;
                }
            }
        }

        if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED))
        {
            drawFrame();
        }
    }

    vkDeviceWaitIdle(device);
}

void VulkanApp::cleanup() 
{
    cleanupSwapChain();

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);

    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyCommandPool(device, graphicsCommandPool, nullptr);
    vkDestroyCommandPool(device, transferCommandPool, nullptr);
    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    
    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

void VulkanApp::run() 
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}