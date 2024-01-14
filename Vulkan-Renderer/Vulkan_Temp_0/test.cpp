// Vulkan��
#include <vulkan/vulkan.h>

// GLFW��
#include <GLFW/glfw3.h>

// C++��
#include <iostream>

// �Զ����
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
    /// һ������
    /// </summary>
    void initWindow()
    {
        // ��ʼ�� GLFW��
        glfwInit();

        // ���ô���
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // ��ʹ��ͼ��API

        // ��������
        myWindow = glfwCreateWindow(const_width, const_height, "Vulkan", nullptr, nullptr);

        // ��������
        glfwSetWindowUserPointer(myWindow, this);
    }

    void initVulkan()
    {
        // ���� Vulkanʵ��
        createVulkanInstance();

        // ���� ͼ����ʾ����
        createSurface();

        // ѡ�� �����豸
        pickPhysicalDevice();

        // ���� �߼��豸
        createLogicalDevice();

        // ���� ��������
        createSwapChain();

        // ���� ͼ����ͼ
        createImageViews();
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(myWindow))
        {
            // �������¼�
            glfwPollEvents();
        }

        vkDeviceWaitIdle(myDevice);
    }

    void cleanup()
    {
        // ���� ���������
        cleanupSwapChain();

        // ���� �߼��豸����
        vkDestroyDevice(myDevice, nullptr);

        // ���� ͼ����ʾ����
        vkDestroySurfaceKHR(myVulkanInstance, mySurface, nullptr);

        // ���� Vulkan ʵ��
        vkDestroyInstance(myVulkanInstance, nullptr);

        // ���� GLFW ����
        glfwDestroyWindow(myWindow);

        // �ͷ� GLFW ��
        glfwTerminate();
    }

    /// <summary>
    /// ��������
    /// </summary>
    void createVulkanInstance()
    {
        // �ж���֤���Ƿ����
        if (enableValidationLayers && !checkValidationLayerSupport()) throw runtime_error("validation layers not available");

        // ��� Ӧ�ó�����Ϣ
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // ��� Vulkanʵ����Ϣ
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // ��� Vulkan��չ��Ϣ
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // ��� ��֤����Ϣ
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

        // ʹ�� VkInstanceCreateInfo ���� VK ʵ��
        if (vkCreateInstance(&createInfo, nullptr, &myVulkanInstance) != VK_SUCCESS) throw runtime_error("failed to create instance!");
    }

    void createSurface()
    {
        // ���� ͼ����ʾ����
        if (glfwCreateWindowSurface(myVulkanInstance, myWindow, nullptr, &mySurface) != VK_SUCCESS)
        {
            throw runtime_error("failed to create window surface!");
        }
    }

    void pickPhysicalDevice()
    {
        // ��ȡ֧�� VK �� GPU ����
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(myVulkanInstance, &deviceCount, nullptr);

        if (deviceCount == 0) throw runtime_error("No GPUs with Vulkan support");

        // ��ȡ֧�� VK �� GPU �б�
        vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(myVulkanInstance, &deviceCount, devices.data());

        // ���� GPU �б�����һ�����ʵ���Ϊ��ǰ GPU
        for (const auto& device : devices)
        {
            if (isDeviceSuitable(device))
            {
                myPhysicalDevice = device;

                // ��� GPU ������
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

        // ��� ���д�����Ϣ
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

        // ��� �豸���ܼ���
        VkPhysicalDeviceFeatures deviceFeatures{};

        // ��� �߼��豸��Ϣ
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // ��� ��֤����Ϣ
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else createInfo.enabledLayerCount = 0;

        // ʹ�� VkDeviceCreateInfo �����߼��豸
        if (vkCreateDevice(myPhysicalDevice, &createInfo, nullptr, &myDevice) != VK_SUCCESS) throw runtime_error("failed to create logical device!");

        // ����ָ��ͼ�ζ��еľ��
        vkGetDeviceQueue(myDevice, indices.graphicsFamily.value(), 0, &myGraphicsQueue);

        // ����ָ����ʾ���еľ��
        vkGetDeviceQueue(myDevice, indices.presentFamily.value(), 0, &myPresentQueue);
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

    void cleanupSwapChain()
    {
        // ���� ͼ����ͼ
        for (auto imageView : mySwapChainImageViews)
        {
            vkDestroyImageView(myDevice, imageView, nullptr);
        }

        // ���� ��������
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