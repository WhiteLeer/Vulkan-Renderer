/// <summary>
/// 
///  ʵ�� | ��֤�� | �����豸 | �߼��豸
/// 
/// </summary>

#include <set>
#include <vector>
#include <optional>
using namespace std;

// ##############################################################

// �Ƿ�����֤��
#define DEBUG 

#ifdef DEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

 // ʹ�� Vulkan SDK ���õ���֤��
const vector<const char*> validationLayers =
{
    "VK_LAYER_KHRONOS_validation"
};

// ʹ�� Vulkan SDK ���õ��豸��չ
const vector<const char*> deviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// �����ж� ������ �Ƿ�֧�� ͼ�ζ��� �� ��ʾ����
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
    // ��ȡ GLFW ���� VK ��չ
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // �����������֤�㣬��� ���Իص� ��չ
    if (enableValidationLayers) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    // ��Ϊ ʵ�ʻص����������������¼�ʱ����
    cerr << "validation layer: " << pCallbackData->pMessage << endl;

    return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    // ��ʼ��
    createInfo = {};

    // �ṹ������
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    // ���ؼ���VERBOSE��WARNING �� ERROR
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    // ��Ϣ���ͣ�GENERAL��VALIDATION �� PERFORMANCE
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    // ����Ļص���������
    createInfo.pfnUserCallback = debugCallback;
}

bool checkValidationLayerSupport()
{
    // ��ȡ��֤����Ϣ
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // �ж� ��ǰ��֤�� �Ƿ������ ������֤�� ��
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

    // ��ȡ֧�ֵĶ����������
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    // ��ȡ֧�ֵĶ�������б�
    vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        // ���֧��ͼ������Ķ���
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;

        // ���֧����ʾ����Ķ���
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
    // ��ȡ �����豸֧�ֵ���չ
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    set<string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    // �ж������豸֧�ֵ���չ�Ƿ�������е� ��������ص���չ
    return requiredExtensions.empty();
}

bool isDeviceSuitable(VkPhysicalDevice device)
{
    // ������֧��
    QueueFamilyIndices indices = findQueueFamilies(device);

    // �����չ֧��
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    return indices.isComplete() && extensionsSupported;
}

// ##############################################################

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