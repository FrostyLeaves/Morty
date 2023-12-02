/**
 * @File         MVulkanPhysicalDevice
 * 
 * @Created      2020-06-17 20:01:48
 *
 * @Author       DoubleYe
**/

#pragma once

#include "MVulkanShaderReflector.h"
#include "Utility/MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include "Render/MIDevice.h"
#include "Render/Vulkan/MVulkanWrapper.h"

#ifdef MORTY_WIN
#include "vulkan/vulkan_core.h"
#endif

class MORTY_API MVulkanPhysicalDevice
{
public:
	explicit MVulkanPhysicalDevice(MEngine* pEngine);


	bool Initialize();
	void Release();

	bool GetDeviceFeatureSupport(MEDeviceFeature feature) const;

	VkPhysicalDevice GetPhysicalDevice() const { return m_VkPhysicalDevice; }

	int FindQueueGraphicsFamilies(VkPhysicalDevice device) const;
	int FindQueuePresentFamilies(VkSurfaceKHR surface) const;
	int FindQueueComputeFamilies(VkPhysicalDevice device) const;
	bool MultiDrawIndirectSupport() const;

	int FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
	VkFormat FindSupportedFormat(const std::set<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
	VkFormatProperties GetFormatProperties(VkFormat vkFormat) const;

	VkImageLayout FindDepthImageLayout() const;

	VkBool32 FormatIsFilterable(VkFormat format, VkImageTiling tiling) const;

	bool CheckVersion(int major, int minor, int patch) const;
	MEngine* GetEngine() const;


	VkDevice CreateLogicalDevice();



protected:
	bool InitVulkanInstance();
	bool InitPhysicalDevice();
	void InitDeviceFeature();
	bool InitDepthFormat();

	bool IsDeviceSuitable(VkPhysicalDevice device) const;
	std::set<MString> GetDeviceOptionFeatureNotSupport(VkPhysicalDevice device) const;
	std::set<MString> GetNotSupportDeviceExtension(VkPhysicalDevice device, const std::set<MString>& tRequiredExtensions) const;

public:
	//instance and device
	VkInstance m_VkInstance = VK_NULL_HANDLE;
	VkPhysicalDevice m_VkPhysicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceFeatures m_VkPhysicalDeviceFeatures;
	VkPhysicalDeviceProperties m_VkPhysicalDeviceProperties = {};
	std::vector<const char*> m_vEnableDeviceExtensions;

	//device family index
	int m_nGraphicsFamilyIndex = 0;
	int m_nComputeFamilyIndex = 0;

	//vulkan version
	int m_nVulkanVersionMajor = 0;
	int m_nVulkanVersionMinor = 0;
	int m_nVulkanVersionPatch = 0;


	//default format.
	VkFormat m_VkDepthTextureFormat = VK_FORMAT_UNDEFINED;
    VkImageAspectFlags m_VkDepthAspectFlags = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout m_VkDepthImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	void* m_pVkExtensionFeaturesList = nullptr;
	std::set<MEDeviceFeature> m_tDisableFeature;
	//optional extensions
	VkPhysicalDeviceVulkan11Features m_VkVulkan11Features{};
	VkPhysicalDeviceConservativeRasterizationPropertiesEXT m_VkConservativeRasterProps{};
	VkPhysicalDeviceFragmentShadingRateFeaturesKHR m_VkShadingRateImageFeatures{};
	
#if MORTY_DEBUG
	VkDebugUtilsMessengerEXT m_VkDebugUtilsMessenger;
#endif



	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = VK_NULL_HANDLE;
	PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2 = VK_NULL_HANDLE;
	PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR = VK_NULL_HANDLE;
	PFN_vkCmdSetFragmentShadingRateKHR vkCmdSetFragmentShadingRateKHR = VK_NULL_HANDLE;

	MEngine* m_pEngine = nullptr;
};


#endif
