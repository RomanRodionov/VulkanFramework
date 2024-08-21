#include "vulkan_app.hpp"
#include "stb_image.h"

void VulkanApp::createImage(VulkanApp::CustomImageCreateInfo& customImageInfo, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = customImageInfo.imageType;
    imageInfo.extent.width = customImageInfo.width;
    imageInfo.extent.height = customImageInfo.height;
    imageInfo.extent.depth = customImageInfo.depth;
    imageInfo.mipLevels = customImageInfo.mipLevels;
    imageInfo.arrayLayers = customImageInfo.arrayLayers;   
    imageInfo.format = customImageInfo.format;
    imageInfo.tiling = customImageInfo.tiling;
    imageInfo.initialLayout = customImageInfo.initialLayout;
    imageInfo.usage = customImageInfo.usage;
    imageInfo.samples = customImageInfo.samples;
    imageInfo.flags = customImageInfo.flags;
    imageInfo.sharingMode = customImageInfo.sharingMode;
    imageInfo.queueFamilyIndexCount = customImageInfo.queueFamilyIndexCount;
    imageInfo.pQueueFamilyIndices = customImageInfo.pQueueFamilyIndices;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, customImageInfo.properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);

}

void VulkanApp::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(transferCommandPool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(transferCommandPool, commandBuffer, transferQueue);
}

void VulkanApp::createTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("resources/textures/floppa.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    QueueFamilyIndices familyIndices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {familyIndices.graphicsFamily.value(), familyIndices.transferFamily.value()};

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    CustomBufferCreateInfo customBufferInfo{};
    customBufferInfo.size = imageSize;
    customBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    customBufferInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    customBufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    customBufferInfo.queueFamilyIndexCount = 2;
    customBufferInfo.pQueueFamilyIndices = queueFamilyIndices;

    createBuffer(customBufferInfo, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    CustomImageCreateInfo customImageInfo{};
    customImageInfo.imageType = VK_IMAGE_TYPE_2D;
    customImageInfo.width = texWidth;
    customImageInfo.height = texHeight;
    customImageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    customImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    customImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    customImageInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    customImageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    customImageInfo.queueFamilyIndexCount = 2;
    customImageInfo.pQueueFamilyIndices = queueFamilyIndices;

    createImage(customImageInfo, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

static bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanApp::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandPool commandPool;
    VkQueue queue;
    if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        commandPool = transferCommandPool;
        queue = transferQueue;
    }
    else
    {
        commandPool = graphicsCommandPool;
        queue = graphicsQueue;
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } 
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } 
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandPool, commandBuffer, queue);
}

void VulkanApp::createImageView(CustomImageViewCreateInfo& customCreateInfo, VkImage& image, VkImageView& imageView)
{
    VkImageViewCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image    = image;
    createInfo.viewType = customCreateInfo.viewType;
    createInfo.format   = customCreateInfo.format;
    createInfo.components.r = customCreateInfo.r;
    createInfo.components.g = customCreateInfo.g;
    createInfo.components.b = customCreateInfo.b;
    createInfo.components.a = customCreateInfo.a;
    createInfo.subresourceRange.aspectMask     = customCreateInfo.aspectMask;
    createInfo.subresourceRange.baseMipLevel   = customCreateInfo.baseMipLevel;
    createInfo.subresourceRange.levelCount     = customCreateInfo.levelCount;
    createInfo.subresourceRange.baseArrayLayer = customCreateInfo.baseArrayLayer;
    createInfo.subresourceRange.layerCount     = customCreateInfo.layerCount;
    
    if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture image view!");
    }
}

void VulkanApp::createTextureImageView()
{
    CustomImageViewCreateInfo customCreateInfo{};
    customCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;

    createImageView(customCreateInfo, textureImage, textureImageView);
}

void VulkanApp::createTextureSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}