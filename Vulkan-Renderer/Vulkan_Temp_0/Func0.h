/// <summary>
/// 
///  ��֤�����
/// 
/// </summary>

#include <vector>
using namespace std;

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