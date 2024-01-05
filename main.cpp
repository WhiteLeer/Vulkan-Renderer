// Vulkan库
#include <vulkan/vulkan.h>

// GLFW库
#include <GLFW/glfw3.h>

// GLM库
#include <glm/glm.hpp>

// C++库
#include <iostream>
#include <fstream>
#include <algorithm>
#include <optional>
#include <vector>
#include <set>
#include <array>

using namespace std;
using namespace glm;

// 是否开启验证层
#define DEBUG 

#ifdef DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

// 使用 Vulkan SDK 内置的验证层
const vector<const char*> validationLayers =
{
    "VK_LAYER_KHRONOS_validation"
};

// 使用 Vulkan SDK 内置的设备拓展
const vector<const char*> deviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// 用于判断 队列组 是否支持 图形队列 和 演示队列
struct QueueFamilyIndices
{
    optional<uint32_t> graphicsFamily;
    optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

// 交换链信息
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{}; // 图像数量、宽高范围
    vector<VkSurfaceFormatKHR> formats{}; // 颜色空间、像素格式
    vector<VkPresentModeKHR> presentModes{}; // 可用演示模式
};

// 顶点信息
struct Vertex
{
    vec2 pos;
    vec3 color;

    // 顶点绑定描述
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    // 顶点属性描述
    static array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

const vector<Vertex> vertices = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

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
    const uint32_t const_width = 800;
    const uint32_t const_height = 600;
    const uint32_t const_maxFrames = 3;

    const std::vector<Vertex> vertices =
    {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    GLFWwindow* myWindow = nullptr;
    VkInstance myVulkanInstance = nullptr;
    VkDebugUtilsMessengerEXT myDebugMessenger = nullptr;
    VkSurfaceKHR mySurface = nullptr;

    VkPhysicalDevice myPhysicalDevice = VK_NULL_HANDLE;
    VkDevice myDevice = nullptr;

    VkQueue myGraphicsQueue = nullptr;
    VkQueue myPresentQueue = nullptr;

    VkSwapchainKHR mySwapChain = nullptr;
    VkFormat mySwapChainImageFormat{};
    VkExtent2D mySwapChainExtent{};

    vector<VkImage> mySwapChainImages{};
    vector<VkImageView> mySwapChainImageViews{};

    VkRenderPass myRenderPass = nullptr;
    VkPipelineLayout myPipelineLayout = nullptr;

    VkPipeline myGraphicsPipeline = nullptr;

    vector<VkFramebuffer> mySwapChainFramebuffers{};

    VkCommandPool myCommandPool = nullptr;
    vector<VkCommandBuffer> myCommandBuffers{};

    vector<VkSemaphore> myImageAvailableSemaphores{};
    vector<VkSemaphore> myRenderFinishedSemaphores{};
    vector<VkFence> myInFlightFences{};

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

        // 设置回调
        glfwSetFramebufferSizeCallback(myWindow, framebufferResizeCallback);
    }

    void initVulkan()
    {
        // 创建 Vulkan实例
        createVulkanInstance();

        // 设置 调试回调对象
        setupDebugMessenger();

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

        // 创建 渲染通道
        createRenderPass();

        // 创建 渲染管线
        createGraphicsPipeline();

        // 创建 帧缓冲区
        createFramebuffers();

        // 创建 命令池
        createCommandPool();

        // 创建 命令缓冲区
        createCommandBuffer();

        // 创建 同步物体
        createSyncObjects();
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(myWindow))
        {
            // 处理窗口事件
            glfwPollEvents();

            // 渲染每一帧
            drawFrame();
        }

        vkDeviceWaitIdle(myDevice);
    }

    void cleanup()
    {
        // 销毁 交换链相关
        cleanupSwapChain();

        // 销毁 渲染管线
        vkDestroyPipeline(myDevice, myGraphicsPipeline, nullptr);

        // 销毁 管线布局
        vkDestroyPipelineLayout(myDevice, myPipelineLayout, nullptr);

        // 销毁 渲染通道
        vkDestroyRenderPass(myDevice, myRenderPass, nullptr);

        // 销毁 信号量、栅栏
        for (int num = 0; num < const_maxFrames; num++)
        {
            vkDestroySemaphore(myDevice, myRenderFinishedSemaphores[num], nullptr);
            vkDestroySemaphore(myDevice, myImageAvailableSemaphores[num], nullptr);
            vkDestroyFence(myDevice, myInFlightFences[num], nullptr);
        }

        // 销毁 命令池
        vkDestroyCommandPool(myDevice, myCommandPool, nullptr);

        // 销毁 逻辑设备引用
        vkDestroyDevice(myDevice, nullptr);

        // 销毁 调试回调对象
        if (enableValidationLayers) DestroyDebugUtilsMessengerEXT(myVulkanInstance, myDebugMessenger, nullptr);

        // 销毁 图像显示对象
        vkDestroySurfaceKHR(myVulkanInstance, mySurface, nullptr);

        // 销毁 Vulkan实例
        vkDestroyInstance(myVulkanInstance, nullptr);

        // 销毁 GLFW窗口
        glfwDestroyWindow(myWindow);

        // 释放 GLFW库
        glfwTerminate();
    }

    /// <summary>
    /// 二级函数
    /// </summary>
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

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

    void setupDebugMessenger()
    {
        if (!enableValidationLayers) return;

        // 如果开启了验证层，调用函数初始化 "调试回调信息"
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        // 使用 "调试回调信息" 创建 "调试回调对象"
        if (CreateDebugUtilsMessengerEXT(myVulkanInstance, &createInfo, nullptr, &myDebugMessenger) != VK_SUCCESS)
        {
            throw runtime_error("failed to set up debug messenger!");
        }
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

    void createRenderPass()
    {
        // 设置 颜色缓冲区描述
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = mySwapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // 设置 颜色缓冲区引用
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // 设置 子通道创建信息
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        // 填充 渲染通道创建信息
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(myDevice, &renderPassInfo, nullptr, &myRenderPass) != VK_SUCCESS)
        {
            throw runtime_error("failed to create render pass!");
        }
    }

    void createGraphicsPipeline()
    {
        // 设置 着色器模块
        auto vertShaderCode = readFile("vert.spv");
        auto fragShaderCode = readFile("frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        // 设置 着色器阶段
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // 设置 顶点输入
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // 设置 图元输入
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // 设置 视口状态
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // 设置 光栅化器
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        // 设置 多重采样
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // 设置 混合方式
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        // 设置 混合参数
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        // 设置 动态状态
        vector<VkDynamicState> dynamicStates =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // 设置 管线布局
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(myDevice, &pipelineLayoutInfo, nullptr, &myPipelineLayout) != VK_SUCCESS)
        {
            throw runtime_error("failed to create pipeline layout!");
        }

        // 创建 渲染管线
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = myPipelineLayout;
        pipelineInfo.renderPass = myRenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(myDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myGraphicsPipeline) != VK_SUCCESS)
        {
            throw runtime_error("failed to create graphics pipeline!");
        }

        // 销毁 着色器模块
        vkDestroyShaderModule(myDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(myDevice, vertShaderModule, nullptr);
    }

    void createFramebuffers()
    {
        mySwapChainFramebuffers.resize(mySwapChainImageViews.size());

        // 为每个 图像视图 创建一个帧缓冲区
        for (size_t i = 0; i < mySwapChainImageViews.size(); i++)
        {
            VkImageView attachments[] =
            {
                mySwapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = myRenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = mySwapChainExtent.width;
            framebufferInfo.height = mySwapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(myDevice, &framebufferInfo, nullptr, &mySwapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createCommandPool()
    {
        // 设置 该命令池的命令全都来自 图形队列
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(myPhysicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(myDevice, &poolInfo, nullptr, &myCommandPool) != VK_SUCCESS)
        {
            throw runtime_error("failed to create command pool!");
        }
    }

    void createCommandBuffer()
    {
        myCommandBuffers.resize(const_maxFrames);

        // 为命令池分配一个命令缓冲区
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = myCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)myCommandBuffers.size();

        if (vkAllocateCommandBuffers(myDevice, &allocInfo, myCommandBuffers.data()) != VK_SUCCESS)
        {
            throw runtime_error("failed to allocate command buffers!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        // 标记 命令缓冲区起点
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw runtime_error("failed to begin recording command buffer!");
        }

        // 初始化 渲染通道
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = myRenderPass;
        renderPassInfo.framebuffer = mySwapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = mySwapChainExtent;

        VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        // 标记 渲染通道起点
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // 绑定 渲染管线
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, myGraphicsPipeline);

        // 设置 渲染视口
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)mySwapChainExtent.width;
        viewport.height = (float)mySwapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // 设置 裁剪矩阵
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = mySwapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // 绘制命令
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        // 标记 渲染通道终点
        vkCmdEndRenderPass(commandBuffer);

        // 标记 命令缓冲区终点
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw runtime_error("failed to record command buffer!");
        }
    }

    void createSyncObjects()
    {
        myImageAvailableSemaphores.resize(const_maxFrames);
        myRenderFinishedSemaphores.resize(const_maxFrames);
        myInFlightFences.resize(const_maxFrames);

        // 用于检测三个信号量
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int num = 0; num < const_maxFrames; num++)
        {
            if (vkCreateSemaphore(myDevice, &semaphoreInfo, nullptr, &myImageAvailableSemaphores[num]) != VK_SUCCESS ||
                vkCreateSemaphore(myDevice, &semaphoreInfo, nullptr, &myRenderFinishedSemaphores[num]) != VK_SUCCESS ||
                vkCreateFence(myDevice, &fenceInfo, nullptr, &myInFlightFences[num]) != VK_SUCCESS)
            {
                throw runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void drawFrame()
    {
        // 等待上一帧完成
        vkWaitForFences(myDevice, 1, &myInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        // 从交换链请求下一张图像
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(myDevice, mySwapChain, UINT64_MAX, myImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        // 当窗口变化时，重新创建交换链
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) throw runtime_error("failed to acquire swap chain image!");

        // 重置限制栅栏信号
        vkResetFences(myDevice, 1, &myInFlightFences[currentFrame]);

        // 重置并记录命令缓冲区
        vkResetCommandBuffer(myCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(myCommandBuffers[currentFrame], imageIndex);

        // 提交命令缓冲区
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { myImageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &myCommandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { myRenderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(myGraphicsQueue, 1, &submitInfo, myInFlightFences[currentFrame]) != VK_SUCCESS)
        {
            throw runtime_error("failed to submit draw command buffer!");
        }

        // 将结果返回交换链
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { mySwapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(myPresentQueue, &presentInfo);

        // 当窗口变化时，重新创建交换链
        // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) 
        // {
        //     framebufferResized = false;
        //     recreateSwapChain();
        // }
        // else if (result != VK_SUCCESS) throw runtime_error("failed to present swap chain image!");

        // 渲染下一帧
        currentFrame = (currentFrame + 1) % const_maxFrames;
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        // 尝试使用内置函数 vkDestroyDebugUtilsMessengerEXT 销毁 "调试回调对象"
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) func(instance, debugMessenger, pAllocator);
    }

    /// <summary>
    /// 三级函数
    /// </summary>
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

    void recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(myWindow, &width, &height);

        // 窗口最小化时暂停
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(myWindow, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(myDevice);

        // 根据新窗口的数据重新创建交换链
        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createFramebuffers();
    }

    bool checkValidationLayerSupport()
    {
        // 获取验证层信息
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // 判断 当前验证层 是否存在于 内置验证层 中
        for (const char* layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) return false;
        }

        return true;
    }

    vector<const char*> getRequiredExtensions()
    {
        // 获取 GLFW 所需 VK 拓展
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        // 如果开启了验证层，添加 调试回调 拓展
        if (enableValidationLayers) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        // 初始化
        createInfo = {};

        // 结构体类型
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        // 严重级别：VERBOSE、WARNING 和 ERROR
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        // 信息类型：GENERAL、VALIDATION 和 PERFORMANCE
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        // 具体的回调函数名称
        createInfo.pfnUserCallback = debugCallback;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        // 尝试使用内置函数 vkCreateDebugUtilsMessengerEXT 创建 "调试回调对象"
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        // 检查队列支持
        QueueFamilyIndices indices = findQueueFamilies(device);

        // 检查拓展支持
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        return indices.isComplete() && extensionsSupported;
    }

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

    VkShaderModule createShaderModule(const vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(myDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    static vector<char> readFile(const string& filename)
    {
        ifstream file(filename, ios::ate | ios::binary);

        if (!file.is_open()) throw runtime_error("failed to open file!");

        size_t fileSize = (size_t)file.tellg();
        vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    /// <summary>
    /// 四级函数
    /// </summary>
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        // 作为 实际回调函数，发生调试事件时调用
        cerr << "validation layer: " << pCallbackData->pMessage << endl;

        return VK_FALSE;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        // 获取支持的队列组的数量
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        // 获取支持的队列组的列表
        vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            // 标记支持图形命令的队列
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;

            // 标记支持演示命令的队列
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mySurface, &presentSupport);
            if (presentSupport) indices.presentFamily = i;

            if (indices.isComplete()) break;

            i++;
        }

        return indices;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        // 获取 物理设备支持的拓展
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        set<string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        // 判断物理设备支持的拓展是否包含所有的 交换链相关的拓展
        return requiredExtensions.empty();
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