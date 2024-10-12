/**
 * @File         MVulkanPhysicalDevice
 * 
 * @Created      2020-06-17 20:01:48
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MVulkanShaderReflector.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "RHI/Abstract/MIDevice.h"
#include "RHI/Vulkan/MVulkanWrapper.h"

#ifdef MORTY_WIN

#include "vulkan/vulkan_core.h"

#endif

namespace morty
{

class MORTY_API MVulkanPhysicalDevice
{
public:
    explicit MVulkanPhysicalDevice(MEngine* pEngine);


    bool             Initialize();

    void             Release();

    bool             GetDeviceFeatureSupport(MEDeviceFeature feature) const;

    VkPhysicalDevice GetPhysicalDevice() const { return m_vkPhysicalDevice; }

    int              FindQueueGraphicsFamilies(VkPhysicalDevice device) const;

    int              FindQueuePresentFamilies(VkSurfaceKHR surface) const;

    int              FindQueueComputeFamilies(VkPhysicalDevice device) const;

    bool             MultiDrawIndirectSupport() const;

    bool             SparseTextureSupport() const;

    bool             ASTCTextureSupport() const;

    bool             BCTextureSupport() const;

    int              FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    VkFormat
    FindSupportedFormat(const std::set<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
            const;

    VkFormatProperties GetFormatProperties(VkFormat vkFormat) const;

    VkImageLayout      FindDepthImageLayout() const;

    VkBool32           FormatIsFilterable(VkFormat format, VkImageTiling tiling) const;

    bool               CheckVersion(int major, int minor, int patch) const;

    MEngine*           GetEngine() const;


    VkDevice           CreateLogicalDevice();


protected:
    bool              InitVulkanInstance();

    bool              InitPhysicalDevice();

    void              InitDeviceFeature();

    bool              InitDepthFormat();

    bool              IsDeviceSuitable(VkPhysicalDevice device) const;

    std::set<MString> GetDeviceOptionFeatureNotSupport(VkPhysicalDevice device) const;

    std::set<MString>
    GetNotSupportDeviceExtension(VkPhysicalDevice device, const std::set<MString>& tRequiredExtensions) const;

public:
    //instance and device
    VkInstance                                             m_vkInstance       = VK_NULL_HANDLE;
    VkPhysicalDevice                                       m_vkPhysicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures                               m_vkPhysicalDeviceFeatures;
    VkPhysicalDeviceProperties                             m_vkPhysicalDeviceProperties = {};
    std::vector<const char*>                               m_enableDeviceExtensions;

    //device family index
    int                                                    m_graphicsFamilyIndex = 0;
    int                                                    m_computeFamilyIndex  = 0;

    //vulkan version
    int                                                    m_vulkanVersionMajor = 0;
    int                                                    m_vulkanVersionMinor = 0;
    int                                                    m_vulkanVersionPatch = 0;


    //default format.
    VkFormat                                               m_vkDepthTextureFormat = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags                                     m_vkDepthAspectFlags   = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout                                          m_vkDepthImageLayout   = VK_IMAGE_LAYOUT_UNDEFINED;

    void*                                                  m_vkExtensionFeaturesList = nullptr;
    std::set<MEDeviceFeature>                              m_disableFeature;
    //optional extensions
    VkPhysicalDeviceVulkan11Features                       m_vkVulkan11Features{};
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT m_vkConservativeRasterProps{};
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR         m_vkShadingRateImageFeatures{};
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR       m_vkFragmentShadingRateProperties{};

#if MORTY_DEBUG
    VkDebugUtilsMessengerEXT m_vkDebugUtilsMessenger;
#endif


    PFN_vkSetDebugUtilsObjectNameEXT      vkSetDebugUtilsObjectNameEXT    = VK_NULL_HANDLE;
    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2  = VK_NULL_HANDLE;
    PFN_vkGetPhysicalDeviceFeatures2KHR   vkGetPhysicalDeviceFeatures2KHR = VK_NULL_HANDLE;
    PFN_vkCmdSetFragmentShadingRateKHR    vkCmdSetFragmentShadingRateKHR  = VK_NULL_HANDLE;

    MEngine*                              m_engine = nullptr;
};

}// namespace morty

#endif
