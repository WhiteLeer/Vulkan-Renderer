/// <summary>
/// 
///  同步物体
/// 
/// </summary>

#include <vector>
using namespace std;

// ##############################################################

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