// Vulkan库
#include <vulkan/vulkan.h>

// GLFW库
#include <GLFW/glfw3.h>

// C++库
#include <iostream>

// 自定义库
#include "Base.h"
#include "Func0.h"
#include "Func1.h"

using namespace std;

class HelloTriangleApplication
{
public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    /// <summary>
    /// 一级函数
    /// </summary>
    void initWindow()
    {
        // 初始化 GLFW库
        glfwInit();

        // 设置窗口
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // 不使用图形API

        // 创建窗口
        myWindow = glfwCreateWindow(const_width, const_height, "Vulkan", nullptr, nullptr);

        // 关联窗口
        glfwSetWindowUserPointer(myWindow, this);
    }

    void initVulkan()
    {
        // 创建 Vulkan实例
        createVulkanInstance();

        // 创建 图像显示对象
        createSurface();

        // 选择 物理设备
        pickPhysicalDevice();

        // 创建 逻辑设备
        createLogicalDevice();

        // 创建 交换链条
        createSwapChain();

        // 创建 图像视图
        createImageViews();
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(myWindow))
        {
            // 处理窗口事件
            glfwPollEvents();
        }

        vkDeviceWaitIdle(myDevice);
    }

    void cleanup()
    {
        // 销毁 交换链相关
        cleanupSwapChain();

        // 销毁 逻辑设备引用
        vkDestroyDevice(myDevice, nullptr);

        // 销毁 图像显示对象
        vkDestroySurfaceKHR(myVulkanInstance, mySurface, nullptr);

        // 销毁 Vulkan 实例
        vkDestroyInstance(myVulkanInstance, nullptr);

        // 销毁 GLFW 窗口
        glfwDestroyWindow(myWindow);

        // 释放 GLFW 库
        glfwTerminate();
    }

    /// <summary>
    /// 二级函数
    /// </summary>
    void createVulkanInstance()
    {
        // 判断验证层是否可用
        if (enableValidationLayers && !checkValidationLayerSupport()) throw runtime_error("validation layers not available");

        // 填充 应用程序信息
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // 填充 Vulkan实例信息
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // 填充 Vulkan拓展信息
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // 填充 验证层信息
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        // 使用 VkInstanceCreateInfo 创建 VK 实例
        if (vkCreateInstance(&createInfo, nullptr, &myVulkanInstance) != VK_SUCCESS) throw runtime_error("failed to create instance!");
    }

    void createSurface()
    {
        // 创建 图像显示对象
        if (glfwCreateWindowSurface(myVulkanInstance, myWindow, nullptr, &mySurface) != VK_SUCCESS)
        {
            throw runtime_error("failed to create window surface!");
        }
    }

    void pickPhysicalDevice()
    {
        // 获取支持 VK 的 GPU 数量
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(myVulkanInstance, &deviceCount, nullptr);

        if (deviceCount == 0) throw runtime_error("No GPUs with Vulkan support");

        // 获取支持 VK 的 GPU 列表
        vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(myVulkanInstance, &deviceCount, devices.data());

        // 遍历 GPU 列表，将第一个合适的作为当前 GPU
        for (const auto& device : devices)
        {
            if (isDeviceSuitable(device))
            {
                myPhysicalDevice = device;

                // 输出 GPU 的名称
                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties(myPhysicalDevice, &deviceProperties);
                cout << "Selected GPU: " << deviceProperties.deviceName << endl;

                break;
            }
        }

        if (myPhysicalDevice == VK_NULL_HANDLE) throw runtime_error("NO suitable GPU");
    }

    void createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(myPhysicalDevice);

        // 填充 队列创建信息
        vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
        set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // 填充 设备功能集成
        VkPhysicalDeviceFeatures deviceFeatures{};

        // 填充 逻辑设备信息
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // 填充 验证层信息
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else createInfo.enabledLayerCount = 0;

        // 使用 VkDeviceCreateInfo 创建逻辑设备
        if (vkCreateDevice(myPhysicalDevice, &createInfo, nullptr, &myDevice) != VK_SUCCESS) throw runtime_error("failed to create logical device!");

        // 保存指向图形队列的句柄
        vkGetDeviceQueue(myDevice, indices.graphicsFamily.value(), 0, &myGraphicsQueue);

        // 保存指向演示队列的句柄
        vkGetDeviceQueue(myDevice, indices.presentFamily.value(), 0, &myPresentQueue);
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

    void cleanupSwapChain()
    {
        // 销毁 图像视图
        for (auto imageView : mySwapChainImageViews)
        {
            vkDestroyImageView(myDevice, imageView, nullptr);
        }

        // 销毁 交换链条
        vkDestroySwapchainKHR(myDevice, mySwapChain, nullptr);
    }
};

int main()
{
    HelloTriangleApplication app;

    try
    {
        app.run();
    }
    catch (const exception& e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}