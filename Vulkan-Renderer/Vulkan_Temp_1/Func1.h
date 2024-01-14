/// <summary>
/// 
///  表面对象 | 交换链 | 图像视图
/// 
/// </summary>

#include <vector>
#include <algorithm>
using namespace std;

// ##############################################################

// 交换链信息
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{}; // 图像数量、宽高范围
    vector<VkSurfaceFormatKHR> formats{}; // 颜色空间、像素格式
    vector<VkPresentModeKHR> presentModes{}; // 可用演示模式
};

// ##############################################################

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(myWindow, &width, &height);

        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    // 获取物理设备上指定 图像显示对象 的基本功能
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mySurface, &details.capabilities);

    // 获取物理设备上指定 图像显示对象 的基本格式
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, mySurface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, mySurface, &formatCount, details.formats.data());
    }

    // 获取物理设备上指定 图像显示对象 的可用演示
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, mySurface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, mySurface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

void cleanupSwapChain()
{
    // 销毁 帧缓冲区
    for (auto framebuffer : mySwapChainFramebuffers)
    {
        vkDestroyFramebuffer(myDevice, framebuffer, nullptr);
    }

    // 销毁 图像视图
    for (auto imageView : mySwapChainImageViews)
    {
        vkDestroyImageView(myDevice, imageView, nullptr);
    }

    // 销毁 交换链条
    vkDestroySwapchainKHR(myDevice, mySwapChain, nullptr);
}

// ##############################################################

void createSurface()
{
    // 创建 表面对象
    if (glfwCreateWindowSurface(myVulkanInstance, myWindow, nullptr, &mySurface) != VK_SUCCESS)
    {
        throw runtime_error("failed to create window surface!");
    }
}

void createSwapChain()
{
    // 获取当前设备在当前环境下支持的属性
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(myPhysicalDevice);

    // 获取指定 基本格式
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);

    // 获取指定 演示模式
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

    // 获取指定 图片宽高
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // 计算 交换链支持的最小图片数
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // 填充 交换链创建信息
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mySurface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // 判断是否同时支持 图形队列 和 演示队列
    QueueFamilyIndices indices = findQueueFamilies(myPhysicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // 使用 "交换链创建信息" 创建 "交换链对象"
    if (vkCreateSwapchainKHR(myDevice, &createInfo, nullptr, &mySwapChain) != VK_SUCCESS)
    {
        throw runtime_error("failed to create swap chain!");
    }

    // 获取当前 与交换链相关的可呈现图像数组
    vkGetSwapchainImagesKHR(myDevice, mySwapChain, &imageCount, nullptr);
    mySwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(myDevice, mySwapChain, &imageCount, mySwapChainImages.data());

    // 获取 可呈现图像数组对应的格式
    mySwapChainImageFormat = surfaceFormat.format;

    // 获取 可呈现图像数组对应的宽高
    mySwapChainExtent = extent;
}

void createImageViews()
{
    mySwapChainImageViews.resize(mySwapChainImages.size());

    // 每个图像视口对应一个图像对象
    for (size_t i = 0; i < mySwapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = mySwapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = mySwapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(myDevice, &createInfo, nullptr, &mySwapChainImageViews[i]) != VK_SUCCESS)
        {
            throw runtime_error("failed to create image views!");
        }
    }
}