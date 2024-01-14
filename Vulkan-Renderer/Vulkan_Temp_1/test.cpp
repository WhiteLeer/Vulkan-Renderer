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
#include "Func2.h"
#include "Func3.h"

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

    void recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(myWindow, &width, &height);

        // ������С��ʱ��ͣ
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(myWindow, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(myDevice);

        // �����´��ڵ��������´���������
        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createFramebuffers();
    }

private:
    uint32_t currentFrame = 0;
    bool framebufferResized = false;

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

        // ���ûص�
        glfwSetFramebufferSizeCallback(myWindow, framebufferResizeCallback);
    }

    void initVulkan()
    {
        // ���� Vulkanʵ��
        createVulkanInstance();

        // ���� �������
        createSurface();

        // ѡ�� �����豸
        pickPhysicalDevice();

        // ���� �߼��豸
        createLogicalDevice();

        // ���� ��������
        createSwapChain();

        // ���� ͼ����ͼ
        createImageViews();

        // ���� ��Ⱦͨ��
        createRenderPass();

        // ���� ��Ⱦ����
        createGraphicsPipeline();

        // ���� ֡������
        createFramebuffers();

        // ���� �����
        createCommandPool();

        // ���� �������
        createCommandBuffer();

        // ���� ͬ������
        createSyncObjects();
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(myWindow))
        {
            // �������¼�
            glfwPollEvents();

            // ��Ⱦÿһ֡
            drawFrame();
        }

        vkDeviceWaitIdle(myDevice);
    }

    void cleanup()
    {
        // ���� ���������
        cleanupSwapChain();

        // ���� ��Ⱦ����
        vkDestroyPipeline(myDevice, myGraphicsPipeline, nullptr);

        // ���� ���߲���
        vkDestroyPipelineLayout(myDevice, myPipelineLayout, nullptr);

        // ���� ��Ⱦͨ��
        vkDestroyRenderPass(myDevice, myRenderPass, nullptr);

        // ���� �ź���
        for (int num = 0; num < const_maxFrames; num++)
        {
            vkDestroySemaphore(myDevice, myRenderFinishedSemaphores[num], nullptr);
            vkDestroySemaphore(myDevice, myImageAvailableSemaphores[num], nullptr);
            vkDestroyFence(myDevice, myInFlightFences[num], nullptr);
        }

        // ���� �����
        vkDestroyCommandPool(myDevice, myCommandPool, nullptr);

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

    void drawFrame()
    {
        // �ȴ���һ֡���
        vkWaitForFences(myDevice, 1, &myInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        // �ӽ�����������һ��ͼ��
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(myDevice, mySwapChain, UINT64_MAX, myImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        // �����ڱ仯ʱ�����´���������
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) throw runtime_error("failed to acquire swap chain image!");

        // ��������դ���ź�
        vkResetFences(myDevice, 1, &myInFlightFences[currentFrame]);

        // ���ò���¼�������
        vkResetCommandBuffer(myCommandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(myCommandBuffers[currentFrame], imageIndex);

        // �ύ�������
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

        // ��������ؽ�����
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { mySwapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(myPresentQueue, &presentInfo);

        // �����ڱ仯ʱ�����´���������
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) 
        {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) throw runtime_error("failed to present swap chain image!");

        // ��Ⱦ��һ֡
        currentFrame = (currentFrame + 1) % const_maxFrames;
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
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