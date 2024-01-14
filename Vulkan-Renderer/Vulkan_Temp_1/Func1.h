/// <summary>
/// 
///  ������� | ������ | ͼ����ͼ
/// 
/// </summary>

#include <vector>
#include <algorithm>
using namespace std;

// ##############################################################

// ��������Ϣ
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{}; // ͼ����������߷�Χ
    vector<VkSurfaceFormatKHR> formats{}; // ��ɫ�ռ䡢���ظ�ʽ
    vector<VkPresentModeKHR> presentModes{}; // ������ʾģʽ
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

    // ��ȡ�����豸��ָ�� ͼ����ʾ���� �Ļ�������
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mySurface, &details.capabilities);

    // ��ȡ�����豸��ָ�� ͼ����ʾ���� �Ļ�����ʽ
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, mySurface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, mySurface, &formatCount, details.formats.data());
    }

    // ��ȡ�����豸��ָ�� ͼ����ʾ���� �Ŀ�����ʾ
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
    // ���� ֡������
    for (auto framebuffer : mySwapChainFramebuffers)
    {
        vkDestroyFramebuffer(myDevice, framebuffer, nullptr);
    }

    // ���� ͼ����ͼ
    for (auto imageView : mySwapChainImageViews)
    {
        vkDestroyImageView(myDevice, imageView, nullptr);
    }

    // ���� ��������
    vkDestroySwapchainKHR(myDevice, mySwapChain, nullptr);
}

// ##############################################################

void createSurface()
{
    // ���� �������
    if (glfwCreateWindowSurface(myVulkanInstance, myWindow, nullptr, &mySurface) != VK_SUCCESS)
    {
        throw runtime_error("failed to create window surface!");
    }
}

void createSwapChain()
{
    // ��ȡ��ǰ�豸�ڵ�ǰ������֧�ֵ�����
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(myPhysicalDevice);

    // ��ȡָ�� ������ʽ
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);

    // ��ȡָ�� ��ʾģʽ
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);

    // ��ȡָ�� ͼƬ���
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // ���� ������֧�ֵ���СͼƬ��
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // ��� ������������Ϣ
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mySurface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // �ж��Ƿ�ͬʱ֧�� ͼ�ζ��� �� ��ʾ����
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

    // ʹ�� "������������Ϣ" ���� "����������"
    if (vkCreateSwapchainKHR(myDevice, &createInfo, nullptr, &mySwapChain) != VK_SUCCESS)
    {
        throw runtime_error("failed to create swap chain!");
    }

    // ��ȡ��ǰ �뽻������صĿɳ���ͼ������
    vkGetSwapchainImagesKHR(myDevice, mySwapChain, &imageCount, nullptr);
    mySwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(myDevice, mySwapChain, &imageCount, mySwapChainImages.data());

    // ��ȡ �ɳ���ͼ�������Ӧ�ĸ�ʽ
    mySwapChainImageFormat = surfaceFormat.format;

    // ��ȡ �ɳ���ͼ�������Ӧ�Ŀ��
    mySwapChainExtent = extent;
}

void createImageViews()
{
    mySwapChainImageViews.resize(mySwapChainImages.size());

    // ÿ��ͼ���ӿڶ�Ӧһ��ͼ�����
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