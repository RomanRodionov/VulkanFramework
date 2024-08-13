#include "vulkan_app.hpp"

uint32_t VulkanApp::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanApp::createBuffer(VulkanApp::CustomBufferCreateInfo& customBufferInfo, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = customBufferInfo.size;
    bufferInfo.usage = customBufferInfo.usage;
    bufferInfo.sharingMode = customBufferInfo.sharingMode;
    bufferInfo.queueFamilyIndexCount = customBufferInfo.queueFamilyIndexCount;
    bufferInfo.pQueueFamilyIndices = customBufferInfo.pQueueFamilyIndices;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed te create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, customBufferInfo.properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }     

    vkBindBufferMemory(device, buffer, bufferMemory, 0);   
}   

void VulkanApp::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = transferCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(transferQueue);

    vkFreeCommandBuffers(device, transferCommandPool, 1, &commandBuffer);
}

void VulkanApp::createVertexBuffer()
{
    QueueFamilyIndices familyIndices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {familyIndices.graphicsFamily.value(), familyIndices.transferFamily.value()};

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    CustomBufferCreateInfo customBufferInfo{};
    customBufferInfo.size = sizeof(vertices[0]) * vertices.size();
    customBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    customBufferInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    customBufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    customBufferInfo.queueFamilyIndexCount = 2;
    customBufferInfo.pQueueFamilyIndices = queueFamilyIndices;

    createBuffer(customBufferInfo, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, customBufferInfo.size, 0, &data);
    memcpy(data, vertices.data(), (size_t) customBufferInfo.size);
    vkUnmapMemory(device, stagingBufferMemory);

    customBufferInfo = {};
    customBufferInfo.size = sizeof(vertices[0]) * vertices.size();
    customBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    customBufferInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    customBufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    customBufferInfo.queueFamilyIndexCount = 2;
    customBufferInfo.pQueueFamilyIndices = queueFamilyIndices;

    createBuffer(customBufferInfo, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, customBufferInfo.size);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}   

void VulkanApp::createIndexBuffer()
{
    QueueFamilyIndices familyIndices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {familyIndices.graphicsFamily.value(), familyIndices.transferFamily.value()};

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    CustomBufferCreateInfo customBufferInfo{};
    customBufferInfo.size = sizeof(indices[0]) * indices.size();
    customBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    customBufferInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    customBufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    customBufferInfo.queueFamilyIndexCount = 2;
    customBufferInfo.pQueueFamilyIndices = queueFamilyIndices;

    createBuffer(customBufferInfo, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, customBufferInfo.size, 0, &data);
    memcpy(data, indices.data(), (size_t) customBufferInfo.size);
    vkUnmapMemory(device, stagingBufferMemory);

    customBufferInfo = {};
    customBufferInfo.size = sizeof(indices[0]) * indices.size();
    customBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    customBufferInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    customBufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
    customBufferInfo.queueFamilyIndexCount = 2;
    customBufferInfo.pQueueFamilyIndices = queueFamilyIndices;

    createBuffer(customBufferInfo, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, customBufferInfo.size);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
} 