#pragma once

#include <vulkan/vulkan.h>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
    }

    bool hasTransferFamily()
    {
        return transferFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct CustomBufferCreateInfo
{
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0; 
    const uint32_t* pQueueFamilyIndices = nullptr;
};

struct CustomImageCreateInfo
{
    VkImageType imageType = VK_IMAGE_TYPE_2D;
    uint32_t width;
    uint32_t height;
    uint32_t depth = 1;
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    VkFormat format;
    VkImageTiling tiling;
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageUsageFlags usage;
    VkMemoryPropertyFlags properties;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageCreateFlags flags = 0;
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0; 
    const uint32_t* pQueueFamilyIndices = nullptr;
};

struct CustomImageViewCreateInfo
{
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    VkFormat format;
    VkComponentSwizzle r = VK_COMPONENT_SWIZZLE_IDENTITY;
    VkComponentSwizzle g = VK_COMPONENT_SWIZZLE_IDENTITY;
    VkComponentSwizzle b = VK_COMPONENT_SWIZZLE_IDENTITY;
    VkComponentSwizzle a = VK_COMPONENT_SWIZZLE_IDENTITY;
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    uint32_t baseMipLevel   = 0;
    uint32_t levelCount     = 1;
    uint32_t baseArrayLayer = 0;
    uint32_t layerCount     = 1;
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};