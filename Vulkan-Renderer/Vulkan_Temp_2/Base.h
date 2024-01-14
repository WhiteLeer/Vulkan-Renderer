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

uint32_t currentFrame = 0;

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

vector<VkDescriptorSet> descriptorSets{};
VkDescriptorPool myDescriptorPool = nullptr;
VkDescriptorSetLayout myDescriptorSetLayout = nullptr;

VkRenderPass myRenderPass = nullptr;
VkPipelineLayout myPipelineLayout = nullptr;

VkPipeline myGraphicsPipeline = nullptr;

vector<VkFramebuffer> mySwapChainFramebuffers{};

VkCommandPool myCommandPool = nullptr;
vector<VkCommandBuffer> myCommandBuffers{};

vector<VkSemaphore> myImageAvailableSemaphores{};
vector<VkSemaphore> myRenderFinishedSemaphores{};
vector<VkFence> myInFlightFences{};

VkBuffer myVertexBuffer = nullptr;
VkDeviceMemory myVertexBufferMemory = nullptr;

VkBuffer myIndexBuffer = nullptr;
VkDeviceMemory myIndexBufferMemory = nullptr;

vector<VkBuffer> myUniformBuffers{};
vector<VkDeviceMemory> myUniformBuffersMemory{};
vector<void*> myUniformBuffersMapped{};

VkImage myTextureImage;
VkSampler myTextureSampler;
VkImageView myTextureImageView;
VkDeviceMemory myTextureImageMemory;