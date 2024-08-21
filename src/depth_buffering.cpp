#include "vulkan_app.hpp"

VkFormat VulkanApp::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanApp::findDepthFormat()
{
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

static bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanApp::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();

    CustomImageCreateInfo customImageInfo{};
    customImageInfo.width = swapChainExtent.width;
    customImageInfo.height = swapChainExtent.height;
    customImageInfo.format = depthFormat;
    customImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    customImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    customImageInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    
    createImage(customImageInfo, depthImage, depthImageMemory);

    CustomImageViewCreateInfo customImageViewInfo{};
    customImageViewInfo.format = depthFormat;
    customImageViewInfo.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    createImageView(customImageViewInfo, depthImage, depthImageView);
}