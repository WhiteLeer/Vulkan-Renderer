/// <summary>
/// 
///  实例 | 验证层 | 物理设备 | 逻辑设备
/// 
/// </summary>

#include <set>
#include <vector>
#include <optional>
using namespace std;

// ##############################################################

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

// ##############################################################

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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    // 作为 实际回调函数，发生调试事件时调用
    cerr << "validation layer: " << pCallbackData->pMessage << endl;

    return VK_FALSE;
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

bool isDeviceSuitable(VkPhysicalDevice device)
{
    // 检查队列支持
    QueueFamilyIndices indices = findQueueFamilies(device);

    // 检查拓展支持
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    return indices.isComplete() && extensionsSupported;
}

// ##############################################################

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