/// <summary>
/// 
///  基本全局变量
/// 
/// </summary>

#include <vector>
using namespace std;

const uint32_t const_width = 800;
const uint32_t const_height = 600;
const uint32_t const_maxFrames = 3;

GLFWwindow* myWindow = nullptr;
VkInstance myVulkanInstance = nullptr;
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