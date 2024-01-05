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

private:
    uint32_t currentFrame = 0;
    bool framebufferResized = false;

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

        // 创建 表面对象
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

        // 销毁 信号量
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

        // 销毁 图像显示对象
        vkDestroySurfaceKHR(myVulkanInstance, mySurface, nullptr);

        // 销毁 Vulkan 实例
        vkDestroyInstance(myVulkanInstance, nullptr);

        // 销毁 GLFW 窗口
        glfwDestroyWindow(myWindow);

        // 释放 GLFW 库
        glfwTerminate();
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
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) 
        {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) throw runtime_error("failed to present swap chain image!");

        // 渲染下一帧
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