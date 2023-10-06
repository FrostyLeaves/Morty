#include "Render/Vulkan/MVulkanDevice.h"
#include "Render/MMesh.h"
#include "Render/MBuffer.h"
#include "Engine/MEngine.h"
#include "Basic/MTexture.h"
#include "Resource/MResource.h"
#include "Utility/MFileHelper.h"
#include "Material/MShaderParam.h"
#include "Render/MVertexBuffer.h"
#include "Render/Vulkan/MVulkanRenderCommand.h"
#include "Utility/MGlobal.h"
#include "vulkan/vulkan_core.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif


#define VALUE_MAX(a, b)(a > b ? a : b)

#if RENDER_GRAPHICS == MORTY_VULKAN

#ifdef MORTY_WIN
#include <windows.h>
#include "vulkan/vulkan_win32.h"
#endif

#ifdef MORTY_ANDROID
#include "vulkan/vulkan_android.h"
#endif

const std::vector<const char*> ValidationLayers = {

	"VK_LAYER_KHRONOS_validation"

};
const std::vector<const char*> DeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_MAINTENANCE1_EXTENSION_NAME,
	VK_KHR_MULTIVIEW_EXTENSION_NAME,
//	VK_GOOGLE_HLSL_FUNCTIONALITY_1_EXTENSION_NAME,
//	VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME,
//	VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME
#if defined(MORTY_WIN) && defined(MORTY_DEBUG)
	VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
#endif
    
#ifdef MORTY_MACOS
    "VK_KHR_portability_subset",
#endif
    
};

const std::set<VkFormat> DepthOnlyTextureFormat = {
	VK_FORMAT_D32_SFLOAT,
};

const std::set<VkFormat> DepthStencilTextureFormat = {
	VK_FORMAT_D32_SFLOAT_S8_UINT,
	VK_FORMAT_D24_UNORM_S8_UINT,
};


const VkImageLayout UndefinedImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

std::vector<const char*> InstanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME,

#ifdef MORTY_DEBUG
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
    
#ifdef MORTY_MACOS
    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
		"VK_EXT_metal_surface",
		"VK_MVK_macos_surface",
#endif

#ifdef MORTY_IOS
		"VK_EXT_metal_surface",
		"VK_MVK_ios_surface",
		"VK_MVK_moltenvk",
#endif

#ifdef MORTY_WIN
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif

#ifdef MORTY_ANDROID
		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#endif

	VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
};

MVulkanDevice::MVulkanDevice()
	: MIDevice()
	, m_ShaderCompiler(this)
    , m_ShaderReflector(this)
	, m_PipelineManager(this)
	, m_BufferPool(this)
{

}

MVulkanDevice::~MVulkanDevice()
{

}

bool MVulkanDevice::Initialize()
{
	if (!InitVulkanInstance())
		return false;

	if (!InitPhysicalDevice())
		return false;

	if (!InitLogicalDevice())
		return false;

	if (!InitCommandPool())
		return false;

	if (!InitDescriptorPool())
		return false;
	
	if (!InitializeRecycleBin())
		return false;
	
	if (!m_BufferPool.Initialize())
		return false;

	if (!InitDefaultTexture())
		return false;

	if (!InitDepthFormat())
		return false;

	InitSampler();

	m_ShaderReflector.Initialize();


//log
	GetEngine()->GetLogger()->Information("Vulkan Validation Extensions:  ");
	for (const char* svExtensionName : ValidationLayers)
	{
		GetEngine()->GetLogger()->Information("   {}", svExtensionName);
	}
	GetEngine()->GetLogger()->Information("\n");

	GetEngine()->GetLogger()->Information("Vulkan Device Extensions:  ");
	for (const char* svExtensionName : DeviceExtensions)
	{
		GetEngine()->GetLogger()->Information("   {}", svExtensionName);
	}
	GetEngine()->GetLogger()->Information("\n");


	GetEngine()->GetLogger()->Information("Vulkan Instance Extensions:  ");
	for (const char* svExtensionName : InstanceExtensions)
	{
		GetEngine()->GetLogger()->Information("    {}", svExtensionName);
	}
	GetEngine()->GetLogger()->Information("\n");


	return true;
}

void MVulkanDevice::Release()
{
	WaitFrameFinish();

	m_PipelineManager.Release();
	m_BufferPool.Release();
	m_ShaderDefaultTexture->DestroyBuffer(this);
	m_ShaderDefaultTexture = nullptr;
	m_ShaderDefaultTextureCube->DestroyBuffer(this);
	m_ShaderDefaultTextureCube = nullptr;
	m_ShaderDefaultTextureArray->DestroyBuffer(this);
	m_ShaderDefaultTextureArray = nullptr;

	for (auto pr : m_tFrameData)
	{
		pr.second.pRecycleBin->Release();
		delete pr.second.pRecycleBin;
		pr.second.pRecycleBin = nullptr;
	}
	m_tFrameData.clear();
	m_pRecycleBin = nullptr;

	m_pDefaultRecycleBin->Release();
	delete m_pDefaultRecycleBin;
	m_pDefaultRecycleBin = nullptr;

#if MORTY_DEBUG
	PFN_vkDestroyDebugUtilsMessengerEXT pvkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_VkInstance, "vkDestroyDebugUtilsMessengerEXT");
	pvkDestroyDebugUtilsMessengerEXT(m_VkInstance, m_VkDebugUtilsMessenger, nullptr);
#endif

	vkDestroySampler(m_VkDevice, m_VkLinearSampler, nullptr);
	vkDestroySampler(m_VkDevice, m_VkNearestSampler, nullptr);
	vkDestroyDescriptorPool(m_VkDevice, m_VkDescriptorPool, nullptr);
	vkDestroyCommandPool(m_VkDevice, m_VkGraphCommandPool, nullptr);
	vkDestroyDevice(m_VkDevice, nullptr);
	vkDestroyInstance(m_VkInstance, NULL);


}

int MVulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_VkPhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	return MGlobal::M_INVALID_INDEX;
}

VkFormat MVulkanDevice::FindSupportedFormat(const std::set<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_VkPhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}

	    if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	return VK_FORMAT_UNDEFINED;
}

VkBool32 MVulkanDevice::FormatIsFilterable(VkFormat format, VkImageTiling tiling)
{
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(m_VkPhysicalDevice, format, &formatProps);

	if (tiling == VK_IMAGE_TILING_OPTIMAL)
		return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	if (tiling == VK_IMAGE_TILING_LINEAR)
		return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

	return false;
}

VkFormat MVulkanDevice::GetFormat(const METextureLayout& layout)
{
	switch (layout)
	{
	case METextureLayout::ER_UNORM_8:
		return VK_FORMAT_R8_UNORM;
		break;
	case METextureLayout::ERG_UNORM_8:
		return VK_FORMAT_R8G8_UNORM;
		break;
	case METextureLayout::ERGB_UNORM_8:
		return VK_FORMAT_R8G8B8_UNORM;
		break;
	case METextureLayout::ERGBA_UNORM_8:
		return VK_FORMAT_R8G8B8A8_UNORM;
		break;

	case METextureLayout::ER_FLOAT_16:
		return VK_FORMAT_R16_SFLOAT;
		break;
	case METextureLayout::ERG_FLOAT_16:
		return VK_FORMAT_R16G16_SFLOAT;
		break;
	case METextureLayout::ERGB_FLOAT_16:
		return VK_FORMAT_R16G16B16_SFLOAT;
		break;
	case METextureLayout::ERGBA_FLOAT_16:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
		break;

	case METextureLayout::ER_FLOAT_32:
		return VK_FORMAT_R32_SFLOAT;
		break;
	case METextureLayout::ERG_FLOAT_32:
		return VK_FORMAT_R32G32_SFLOAT;
		break;
	case METextureLayout::ERGB_FLOAT_32:
		return VK_FORMAT_R32G32B32_SFLOAT;
		break;
	case METextureLayout::ERGBA_FLOAT_32:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
		break;
	case METextureLayout::EDepth:
		return m_VkDepthTextureFormat;
		break;

	default:
		return VK_FORMAT_R8G8B8A8_SRGB;
		break;
	}

	return VK_FORMAT_R8G8B8A8_SRGB;
}

VkImageUsageFlags MVulkanDevice::GetUsageFlags(MTexture* pTexture)
{
	VkImageUsageFlags usageFlags = 0;
	if (pTexture->GetShaderUsage() == METextureShaderUsage::ESampler)
	{
		usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderBack)
	{
		usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	else if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderPresent)
	{
		usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	else if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderDepth)
	{
		usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	if (pTexture->GetReadable())
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	return usageFlags;
}

VkImageAspectFlags MVulkanDevice::GetAspectFlags(MTexture* pTexture)
{
	if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderDepth)
	{
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	return VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageAspectFlags MVulkanDevice::GetAspectFlags(VkFormat format)
{
	if (DepthStencilTextureFormat.find(format) != DepthStencilTextureFormat.end())
	{
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	if (DepthOnlyTextureFormat.find(format) != DepthOnlyTextureFormat.end())
	{
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	return VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageLayout MVulkanDevice::GetImageLayout(MTexture* pTexture)
{
	if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderPresent)
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderDepth)
	{
        //VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL support after vulkan 1.2.0
        if (!CheckVersion(1, 2, 0))
        {
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        
        if (m_VkDepthAspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT)
        {
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
		
        return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	}
	if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderBack)
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	if (pTexture->GetShaderUsage() == METextureShaderUsage::ESampler)
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkImageViewType MVulkanDevice::GetImageViewType(MTexture* pTexture)
{
	if (METextureType::ETextureCube == pTexture->GetTextureType())
		return VK_IMAGE_VIEW_TYPE_CUBE;
	else if (METextureType::ETexture2DArray == pTexture->GetTextureType())
		return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	else if (METextureType::ETexture3D == pTexture->GetTextureType())
		return VK_IMAGE_VIEW_TYPE_3D;

	return VK_IMAGE_VIEW_TYPE_2D;
}

VkImageCreateFlags MVulkanDevice::GetImageCreateFlags(MTexture* pTexture)
{
	if (pTexture->GetTextureType() == METextureType::ETextureCube)
		return VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	else if (pTexture->GetTextureType() == METextureType::ETexture2DArray)
		return 0;

	return 0;
}

VkImageType MVulkanDevice::GetImageType(MTexture* pTexture)
{
	if (pTexture->GetTextureType() == METextureType::ETextureCube)
		return VK_IMAGE_TYPE_2D;
	else if (pTexture->GetTextureType() == METextureType::ETexture2DArray)
		return VK_IMAGE_TYPE_2D;
	else if (pTexture->GetTextureType() == METextureType::ETexture3D)
		return VK_IMAGE_TYPE_3D;

	return VK_IMAGE_TYPE_2D;
}

int MVulkanDevice::GetMipmapCount(MTexture* pTexture)
{
	uint32_t unMipmap = 1;

	if (pTexture->GetMipmapsEnable())
	{
		unMipmap = static_cast<uint32_t>(std::floor(std::log2(std::max(pTexture->GetSize().x, pTexture->GetSize().y)))) + 1;
	}

	return unMipmap;
}

int MVulkanDevice::GetLayerCount(MTexture* pTexture)
{
	if (!pTexture)
		return 0;

	return pTexture->GetImageLayerNum();
}

MVulkanObjectRecycleBin* MVulkanDevice::GetRecycleBin()
{
	if (m_pRecycleBin)
		return m_pRecycleBin;
	
	return m_pDefaultRecycleBin;
}

void MVulkanDevice::SetDebugName(uint64_t object, const VkObjectType& type, const char* svDebugName)
{
#if MORTY_DEBUG
	VkDebugUtilsObjectNameInfoEXT vkObjectName;
	vkObjectName.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	vkObjectName.objectType = type;
	vkObjectName.pNext = nullptr;
	vkObjectName.objectHandle = object;
	vkObjectName.pObjectName = svDebugName;
	vkSetDebugUtilsObjectNameEXT(m_VkDevice, &vkObjectName);
#endif
}

bool MVulkanDevice::CheckVersion(int major, int minor, int patch)
{
    if (major == m_nVulkanVersionMajor)
    {
        if (minor == m_nVulkanVersionMinor)
        {
            return patch <= m_nVulkanVersionPatch;
        }
        return minor < m_nVulkanVersionMinor;
    }

    return major < m_nVulkanVersionMajor;
}

VkBool32 VKAPI_PTR OutputDebugUtilsMessenger(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	MORTY_UNUSED(messageType);
	
	if (VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT == messageSeverity)
	{
		MVulkanDevice* pDevice = static_cast<MVulkanDevice*>(pUserData);
		pDevice->GetEngine()->GetLogger()->Error(pCallbackData->pMessage);
		MORTY_ASSERT(false);
	}
	return VK_FALSE;
}

bool MVulkanDevice::InitDepthFormat()
{
	constexpr VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkFormat format = FindSupportedFormat(DepthStencilTextureFormat, VK_IMAGE_TILING_OPTIMAL, features);

	if (format != VK_FORMAT_UNDEFINED)
	{
		m_VkDepthTextureFormat = format;
        m_VkDepthAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		return true;
	}

	format = FindSupportedFormat(DepthOnlyTextureFormat, VK_IMAGE_TILING_OPTIMAL, features);
	if (format != VK_FORMAT_UNDEFINED)
    {
		m_VkDepthTextureFormat = format;
        m_VkDepthAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		return true;
	}

	return false;
}

bool MVulkanDevice::InitSampler()
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

	if (vkCreateSampler(m_VkDevice, &samplerInfo, nullptr, &m_VkLinearSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	if (vkCreateSampler(m_VkDevice, &samplerInfo, nullptr, &m_VkNearestSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	return true;
}

bool MVulkanDevice::InitializeRecycleBin()
{
	m_pDefaultRecycleBin = new MVulkanObjectRecycleBin(this);
	m_pDefaultRecycleBin->Initialize();
	return true;
}

bool MVulkanDevice::InitDescriptorPool()
{
	uint32_t unSwapChainNum = 50000;

	std::vector<VkDescriptorPoolSize> vPoolSize = {
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			unSwapChainNum,
		},
		{
			VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			unSwapChainNum,
		},
		{
			VK_DESCRIPTOR_TYPE_SAMPLER,
			unSwapChainNum,
		},
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			unSwapChainNum,
		},
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			unSwapChainNum,
		},
		{
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			unSwapChainNum,
		},
		{
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
			unSwapChainNum,
		},
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.poolSizeCount = vPoolSize.size();
	poolInfo.pPoolSizes = vPoolSize.data();

	poolInfo.maxSets = unSwapChainNum;

	if (vkCreateDescriptorPool(m_VkDevice, &poolInfo, nullptr, &m_VkDescriptorPool) != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

void MVulkanDevice::GenerateBuffer(MBuffer* pBuffer, const MByte* initialData, const size_t& unDataSize)
{
	VkDeviceSize unBufferSize = static_cast<uint64_t>(pBuffer->GetSize());

	void* pMapMemory = nullptr;
	VkBuffer vkBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;

	VkBufferUsageFlags vkBufferUsageFlags = GetBufferUsageFlags(pBuffer);
	VkMemoryPropertyFlags vkMemoryFlags = GetMemoryFlags(pBuffer);

	if (MBuffer::MMemoryType::EHostVisible == pBuffer->m_eMemoryType)
	{
		GenerateBuffer(unBufferSize, vkBufferUsageFlags, vkMemoryFlags, vkBuffer, vkDeviceMemory);

		if (initialData && unDataSize >= unBufferSize)
		{
			vkMapMemory(m_VkDevice, vkDeviceMemory, 0, unBufferSize, 0, &pMapMemory);
			memcpy(pMapMemory, initialData, static_cast<size_t>(unBufferSize));
			vkUnmapMemory(m_VkDevice, vkDeviceMemory);
		}
	}
	else if (MBuffer::MMemoryType::EDeviceLocal == pBuffer->m_eMemoryType)
	{
		GenerateBuffer(unBufferSize, vkBufferUsageFlags, vkMemoryFlags, vkBuffer, vkDeviceMemory);

		if (initialData && unDataSize >= unBufferSize)
		{
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			GenerateBuffer(unBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			vkMapMemory(m_VkDevice, stagingBufferMemory, 0, unBufferSize, 0, &pMapMemory);
			memcpy(pMapMemory, initialData, (size_t)unBufferSize);
			vkUnmapMemory(m_VkDevice, stagingBufferMemory);

			VkBufferCopy region = { 0, 0, unBufferSize };
			CopyBuffer(stagingBuffer, vkBuffer, region);

			DestroyBuffer(stagingBuffer, stagingBufferMemory);
		}
	}
	else
	{
		MORTY_ASSERT(false);
	}

#ifdef MORTY_DEBUG
	SetDebugName(reinterpret_cast<uint64_t>(vkBuffer), VkObjectType::VK_OBJECT_TYPE_BUFFER, pBuffer->GetDebugName());
#endif

	pBuffer->m_VkBuffer = vkBuffer;
	pBuffer->m_VkDeviceMemory = vkDeviceMemory;
	pBuffer->m_eStageType = MBuffer::MStageType::ESynced;
}

void MVulkanDevice::DownloadBuffer(MBuffer* pBuffer, MByte* outputData, const size_t& nSize)
{
	if (MBuffer::MMemoryType::EHostVisible == pBuffer->m_eMemoryType)
	{
		void* pMapMemory = nullptr;
		vkMapMemory(m_VkDevice, pBuffer->m_VkDeviceMemory, 0, nSize, 0, &pMapMemory);
		memcpy(outputData, pMapMemory, static_cast<size_t>(nSize));
		vkUnmapMemory(m_VkDevice, pBuffer->m_VkDeviceMemory);
	}
    else
    {
		MORTY_ASSERT(false);
    }
}

void MVulkanDevice::DestroyBuffer(MBuffer* pBuffer)
{
	if(!pBuffer)
	{
		return;
	}

	GetRecycleBin()->DestroyBufferLater(pBuffer->m_VkBuffer);
	GetRecycleBin()->DestroyDeviceMemoryLater(pBuffer->m_VkDeviceMemory);

	pBuffer->m_eStageType = MBuffer::MStageType::EUnknow;
}

void MVulkanDevice::UploadBuffer(MBuffer* pBuffer, const size_t& unBeginOffset, const MByte* data, const size_t& unDataSize)
{
	if(!pBuffer)
	{
		return;
	}
	
	if (MBuffer::MMemoryType::EHostVisible == pBuffer->m_eMemoryType)
	{
		size_t unMappingSize = (std::min)(unDataSize, pBuffer->GetSize() - unBeginOffset);
		void* dataMapping = nullptr;
		vkMapMemory(m_VkDevice, pBuffer->m_VkDeviceMemory, unBeginOffset, unMappingSize, 0, &dataMapping);
		memcpy(dataMapping, data, unMappingSize);
		vkUnmapMemory(m_VkDevice, pBuffer->m_VkDeviceMemory);

		pBuffer->m_eStageType = MBuffer::MStageType::ESynced;
	}
	else
	{
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		void* dataMapping = nullptr;
		GenerateBuffer(unDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		vkMapMemory(m_VkDevice, stagingBufferMemory, 0, unDataSize, 0, &dataMapping);
		memcpy(dataMapping, data, (size_t)unDataSize);
		vkUnmapMemory(m_VkDevice, stagingBufferMemory);

		const VkBufferCopy region = { 0, unBeginOffset, unDataSize };
		CopyBuffer(stagingBuffer, pBuffer->m_VkBuffer, region);

		DestroyBuffer(stagingBuffer, stagingBufferMemory);
	}
}

void MVulkanDevice::GenerateTexture(MTexture* pTexture, const MByte* pData)
{
	uint32_t width = std::max(static_cast<int>(pTexture->GetSize().x), 1);
	uint32_t height = std::max(static_cast<int>(pTexture->GetSize().y), 1);
    VkFormat format = GetFormat(pTexture->GetTextureLayout());

	VkImageUsageFlags usageFlags = GetUsageFlags(pTexture);
	VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkImageAspectFlags aspectFlgas = GetAspectFlags(pTexture);
	VkImageLayout defaultLayout = UndefinedImageLayout;

	VkImage textureImage = VK_NULL_HANDLE;
	VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
	VkImageCreateFlags createFlags = GetImageCreateFlags(pTexture);
	VkImageType imageType = GetImageType(pTexture);
	int unMipmap = GetMipmapCount(pTexture);
	uint32_t unLayerCount = GetLayerCount(pTexture);

	VkDeviceSize imageSize = static_cast<uint64_t>(MTexture::GetImageMemorySize(pTexture->GetTextureLayout())) * width * height * unLayerCount;

	if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderPresent)
	{
		VkImageSubresourceRange vkSubresourceRange = {};
		vkSubresourceRange.aspectMask = aspectFlgas;
		vkSubresourceRange.baseMipLevel = 0;
		vkSubresourceRange.levelCount = unMipmap;
		vkSubresourceRange.layerCount = unLayerCount;
		TransitionImageLayout(pTexture->m_VkTextureImage, UndefinedImageLayout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vkSubresourceRange);


		pTexture->m_unMipmapLevel = 1;
	}
	else
	{
		if (pTexture->m_VkTextureImage)
		{
			MORTY_ASSERT(false);
		}

		CreateImage(width, height, unMipmap, unLayerCount, format, VK_IMAGE_TILING_OPTIMAL, usageFlags, memoryFlags, defaultLayout, textureImage, textureImageMemory, createFlags, imageType);

		if (pData && pTexture->GetShaderUsage() == METextureShaderUsage::ESampler)
		{
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			GenerateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

			MByte* data;
			vkMapMemory(m_VkDevice, stagingBufferMemory, 0, imageSize, 0, (void**)&data);
			memcpy(data, pData, static_cast<size_t>(imageSize));
			vkUnmapMemory(m_VkDevice, stagingBufferMemory);

			VkImageSubresourceRange vkSubresourceRange = {};
			vkSubresourceRange.aspectMask = aspectFlgas;
			vkSubresourceRange.baseMipLevel = 0;
			vkSubresourceRange.levelCount = unMipmap;
			vkSubresourceRange.layerCount = unLayerCount;

			VkCommandBuffer commandBuffer = BeginCommands();
			TransitionImageLayout(commandBuffer, textureImage, UndefinedImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkSubresourceRange);

			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = unLayerCount;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };

			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			TransitionImageLayout(commandBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vkSubresourceRange);
			EndCommands(commandBuffer);

			DestroyBuffer(stagingBuffer, stagingBufferMemory);
		}
		else
		{
			VkImageSubresourceRange vkSubresourceRange = {};
			vkSubresourceRange.aspectMask = GetAspectFlags(format);
			vkSubresourceRange.baseMipLevel = 0;
			vkSubresourceRange.levelCount = unMipmap;
			vkSubresourceRange.layerCount = unLayerCount;
			TransitionImageLayout(textureImage, UndefinedImageLayout, GetImageLayout(pTexture), vkSubresourceRange);
		}

		pTexture->m_VkTextureImage = textureImage;
		pTexture->m_unMipmapLevel = static_cast<uint32_t>(unMipmap);
		pTexture->m_VkTextureImageMemory = textureImageMemory;
		pTexture->m_VkTextureFormat = format;
		pTexture->m_VkImageLayout = GetImageLayout(pTexture);
	}


	pTexture->m_VkImageView = CreateImageView(pTexture->m_VkTextureImage, pTexture->m_VkTextureFormat, aspectFlgas, unMipmap, unLayerCount, GetImageViewType(pTexture));
	

	if (unMipmap > 1)
	{
		GenerateMipmaps(pTexture, unMipmap);
	}

// 	if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderBack)
// 	{
// 		VkImageSubresourceRange vkSubresourceRange = {};
// 		vkSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 		vkSubresourceRange.baseMipLevel = 0;
// 		vkSubresourceRange.levelCount = unMipmap;
// 		vkSubresourceRange.layerCount = 1;
// 		TransitionImageLayout(pTexture->m_VkTextureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, vkSubresourceRange);
// 	}
	if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderDepth)
	{
		VkFilter vkShadowMapFilter = FormatIsFilterable(format, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

		VkSampler depthSampler;
		VkSamplerCreateInfo sampler = {};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.magFilter = vkShadowMapFilter;
		sampler.minFilter = vkShadowMapFilter;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeV = sampler.addressModeU;
		sampler.addressModeW = sampler.addressModeU;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 1.0f;
		sampler.minLod = 0.0f;
		sampler.maxLod = 1.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		vkCreateSampler(m_VkDevice, &sampler, nullptr, &depthSampler);

		pTexture->m_VkSampler = depthSampler;
	}


	MORTY_ASSERT(!pTexture->GetName().empty());
	if (!pTexture->GetName().empty())
	{
		SetDebugName((uint64_t)pTexture->m_VkTextureImage, VkObjectType::VK_OBJECT_TYPE_IMAGE, pTexture->GetName().c_str());
		SetDebugName((uint64_t)pTexture->m_VkImageView, VkObjectType::VK_OBJECT_TYPE_IMAGE_VIEW, pTexture->GetName().c_str());
		if (pTexture->m_VkTextureImageMemory)
		{
			SetDebugName((uint64_t)pTexture->m_VkTextureImageMemory, VkObjectType::VK_OBJECT_TYPE_DEVICE_MEMORY, pTexture->GetName().c_str());
		}
	}

}

void MVulkanDevice::DestroyTexture(MTexture* pTexture)
{
	if (pTexture->m_VkImageView)
	{
		GetRecycleBin()->DestroyImageViewLater(pTexture->m_VkImageView);
		pTexture->m_VkImageView = VK_NULL_HANDLE;
	}

	if (pTexture->GetRenderUsage() != METextureRenderUsage::ERenderPresent && pTexture->m_VkTextureImage)
	{
		GetRecycleBin()->DestroyImageLater(pTexture->m_VkTextureImage);
		pTexture->m_VkTextureImage = VK_NULL_HANDLE;
	}

	if (pTexture->m_VkTextureImageMemory)
	{
		GetRecycleBin()->DestroyDeviceMemoryLater(pTexture->m_VkTextureImageMemory);
		pTexture->m_VkTextureImageMemory = VK_NULL_HANDLE;
	}

	if (pTexture->m_VkSampler)
	{
		GetRecycleBin()->DestroySamplerLater(pTexture->m_VkSampler);
		pTexture->m_VkSampler = VK_NULL_HANDLE;
	}
}

bool MVulkanDevice::CompileShader(MShader* pShader)
{
	if (!pShader)
		return false;

	std::vector<uint32_t> spirv;
	m_ShaderCompiler.CompileShader(pShader->GetShaderPath(), pShader->GetType(), pShader->GetMacro(), spirv);

	if (spirv.size() == 0)
		return false;

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = spirv.size() * sizeof(uint32_t);
	createInfo.pCode = reinterpret_cast<const uint32_t*>(spirv.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(m_VkDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		return false;

	spirv_cross::Compiler compiler(spirv);

	VkPipelineShaderStageCreateInfo shaderStageInfo{};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo.module = shaderModule;

	if (pShader->GetType() == MEShaderType::EVertex)
	{
		shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStageInfo.pName = "VS_MAIN";
	}
	else if (pShader->GetType() == MEShaderType::EPixel)
	{
		shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStageInfo.pName = "PS_MAIN";
	}
	else if (pShader->GetType() == MEShaderType::ECompute)
	{
		shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStageInfo.pName = "CS_MAIN";
	}
	else if (pShader->GetType() == MEShaderType::EGeometry)
	{
		shaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		shaderStageInfo.pName = "GS_MAIN";
	}
	else
	{
		MORTY_ASSERT(false);
	}



	shaderStageInfo.pSpecializationInfo = nullptr;

	MShaderBuffer* pShaderBuffer = nullptr;
	if (MEShaderType::EVertex == pShader->GetType())
	{
		MVertexShaderBuffer* pBuffer = new MVertexShaderBuffer();
		m_ShaderReflector.GetVertexInputState(compiler, pBuffer);
		pShaderBuffer = pBuffer;
	}
	else if (MEShaderType::EPixel == pShader->GetType())
	{
		MPixelShaderBuffer* pBuffer = new MPixelShaderBuffer();
		pShaderBuffer = pBuffer;
	}
	else if (MEShaderType::ECompute == pShader->GetType())
	{
		MComputeShaderBuffer* pBuffer = new MComputeShaderBuffer();
		pShaderBuffer = pBuffer;
	}
	else if (MEShaderType::EGeometry == pShader->GetType())
	{
		MGeometryShaderBuffer* pBuffer = new MGeometryShaderBuffer();
		pShaderBuffer = pBuffer;
	}
	else
	{
		MORTY_ASSERT(false);
	}


	pShaderBuffer->m_VkShaderModule = shaderModule;
	pShaderBuffer->m_VkShaderStageInfo = shaderStageInfo;
	m_ShaderReflector.GetShaderParam(compiler, pShaderBuffer);

	pShader->SetBuffer(pShaderBuffer);
	return true;
}

void MVulkanDevice::CleanShader(MShader* pShader)
{
	if (!pShader)
		return;

	MShaderBuffer* pBuffer = pShader->GetBuffer();
	if (!pBuffer)
		return;

	GetRecycleBin()->DestroyShaderModuleLater(pBuffer->m_VkShaderModule);

	delete pBuffer;
	pShader->SetBuffer(nullptr);
}

bool MVulkanDevice::GenerateShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock)
{
	if (!pPropertyBlock)
		return false;
	
	return true;
}

void MVulkanDevice::DestroyShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pPropertyBlock)
{
	if (pPropertyBlock)
	{
		m_PipelineManager.DestroyShaderPropertyBlock(pPropertyBlock);
	}
}

bool MVulkanDevice::GenerateShaderParamBuffer(const std::shared_ptr<MShaderConstantParam>& pParam)
{
	if (!pParam)
		return false;

	if (VK_NULL_HANDLE != pParam->m_VkBuffer)
		DestroyShaderParamBuffer(pParam);


	return m_BufferPool.AllowBufferMemory(pParam);
}

void MVulkanDevice::DestroyShaderParamBuffer(const std::shared_ptr<MShaderConstantParam>& pParam)
{
	if (!pParam)
		return;

	
	m_BufferPool.FreeBufferMemory(pParam);
}

bool MVulkanDevice::GenerateRenderPass(MRenderPass* pRenderPass)
{
	if (!pRenderPass)
		return false;

	if (VK_NULL_HANDLE != pRenderPass->m_VkRenderPass)
	{
		DestroyRenderPass(pRenderPass);
	}

	uint32_t unBackNum = pRenderPass->m_vBackTextures.size();

	std::vector<VkAttachmentDescription> vAttachmentDesc;

	for (uint32_t i = 0; i < unBackNum; ++i)
	{
		MRenderTarget& backTexture = pRenderPass->m_vBackTextures[i];
		if (!backTexture.pTexture)
		{
			GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateRenderPass error: bt == nullptr");
			return false;
		}

		if (backTexture.pTexture->m_VkTextureImage == VK_NULL_HANDLE)
		{
			backTexture.pTexture->GenerateBuffer(this);
		}
		
		MORTY_ASSERT(backTexture.desc.bClearWhenRender || backTexture.desc.bAlreadyRender);

		vAttachmentDesc.push_back({});
		VkAttachmentDescription& colorAttachment = vAttachmentDesc.back();


		if (backTexture.desc.bClearWhenRender)
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		else
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

		colorAttachment.format = backTexture.pTexture->m_VkTextureFormat;

		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		if (backTexture.pTexture->GetRenderUsage() == METextureRenderUsage::ERenderPresent)
		{
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
		else
		{
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		if (backTexture.desc.bAlreadyRender)
			colorAttachment.initialLayout = colorAttachment.finalLayout;
		else
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	}

	std::shared_ptr<MTexture> pDepthTexture = pRenderPass->GetDepthTexture();
	if (pDepthTexture)
	{
		if (pDepthTexture->m_VkTextureImage == VK_NULL_HANDLE)
		{
			pDepthTexture->GenerateBuffer(this);
		}

		vAttachmentDesc.push_back({});
		VkAttachmentDescription& colorAttachment = vAttachmentDesc.back();

		if (pRenderPass->m_DepthTexture.desc.bClearWhenRender)
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		else
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

		colorAttachment.format = pDepthTexture->m_VkTextureFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		if (pDepthTexture->GetShaderUsage() == METextureShaderUsage::ESampler)
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		else
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;			//for Shader

		if (pRenderPass->m_DepthTexture.desc.bAlreadyRender)
			colorAttachment.initialLayout = colorAttachment.finalLayout;
		else
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}


	std::vector<VkSubpassDescription> vSubpass;

	std::vector<std::vector<VkAttachmentReference>> vOutAttachmentRef;
	std::vector<std::vector<VkAttachmentReference>> vOutDepthAttachmentRef;
	std::vector<std::vector<VkAttachmentReference>> vInAttachmentRef;
	std::vector<std::vector<uint32_t>> vUnusedAttachmentRef;
	std::vector<uint32_t> vViewMask;
	std::vector<uint32_t> vCorrelationMask;

	// m_ShaderDefaultTexture subpass
	if (pRenderPass->m_vSubpass.empty())
	{
		vOutAttachmentRef.resize(1);
		vOutDepthAttachmentRef.resize(1);
		vViewMask.push_back((1 << pRenderPass->GetViewportNum()) - 1);
		vCorrelationMask.push_back((1 << pRenderPass->GetViewportNum()) - 1);

		vSubpass.push_back(VkSubpassDescription());
		VkSubpassDescription& vkSubpass = vSubpass.back();
		vkSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		for (uint32_t i = 0; i < unBackNum; ++i)
			vOutAttachmentRef[0].push_back({ uint32_t(vOutAttachmentRef[0].size()), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

		vkSubpass.colorAttachmentCount = unBackNum;

		if (pDepthTexture)
		{
			vOutDepthAttachmentRef[0] = { { uint32_t(vOutAttachmentRef[0].size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL } };
			vkSubpass.pDepthStencilAttachment = vOutDepthAttachmentRef[0].data();
		}

		vkSubpass.pColorAttachments = vOutAttachmentRef[0].data();
	}
	else  //
	{
		uint32_t unSubpassNum = pRenderPass->m_vSubpass.size();
		vOutAttachmentRef.resize(unSubpassNum);
		vOutDepthAttachmentRef.resize(unSubpassNum);
		vInAttachmentRef.resize(unSubpassNum);
		vUnusedAttachmentRef.resize(unSubpassNum);
		vViewMask.resize(unSubpassNum);
		vCorrelationMask.resize(unSubpassNum);

		for (uint32_t nSubpassIdx = 0; nSubpassIdx < unSubpassNum; ++nSubpassIdx)
		{
			MSubpass& subpass = pRenderPass->m_vSubpass[nSubpassIdx];

			vSubpass.push_back(VkSubpassDescription());
			VkSubpassDescription& vkSubpass = vSubpass.back();
			vkSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

			vViewMask[nSubpassIdx] = subpass.m_unViewMask;
			vCorrelationMask[nSubpassIdx] = subpass.m_unCorrelationMask;

			std::set<uint32_t> vUsedAttachIndex;

			for (uint32_t i = 0; i < subpass.m_vOutputIndex.size(); ++i)
			{
				uint32_t nBackIdx = subpass.m_vOutputIndex[i];

				vOutAttachmentRef[nSubpassIdx].push_back({});
				VkAttachmentReference& vkAttachRef = vOutAttachmentRef[nSubpassIdx].back();

				vkAttachRef.attachment = nBackIdx;
				vkAttachRef.layout = VK_IMAGE_LAYOUT_GENERAL;

				vUsedAttachIndex.insert(nBackIdx);
			}

			if (pDepthTexture)
			{
				vOutDepthAttachmentRef[nSubpassIdx] = { { uint32_t(vOutAttachmentRef[nSubpassIdx].size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL } };
				vkSubpass.pDepthStencilAttachment = vOutDepthAttachmentRef[nSubpassIdx].data();
			}

			vkSubpass.colorAttachmentCount = vOutAttachmentRef[nSubpassIdx].size();
			vkSubpass.pColorAttachments = vOutAttachmentRef[nSubpassIdx].data();

			for (uint32_t i = 0; i < subpass.m_vInputIndex.size(); ++i)
			{
				uint32_t nBackIdx = subpass.m_vInputIndex[i];

				vInAttachmentRef[nSubpassIdx].push_back({});
				VkAttachmentReference& vkAttachRef = vInAttachmentRef[nSubpassIdx].back();

				vkAttachRef.attachment = nBackIdx;
				vkAttachRef.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				vUsedAttachIndex.insert(nBackIdx);
			}

			vkSubpass.inputAttachmentCount = vInAttachmentRef[nSubpassIdx].size();
			vkSubpass.pInputAttachments = vInAttachmentRef[nSubpassIdx].data();

			for (uint32_t i = 0; i < unBackNum; ++i)
			{
				if (vUsedAttachIndex.find(i) == vUsedAttachIndex.end())
				{
					vUnusedAttachmentRef[nSubpassIdx].push_back(i);
				}
			}

			vkSubpass.preserveAttachmentCount = vUnusedAttachmentRef[nSubpassIdx].size();
			vkSubpass.pPreserveAttachments = vUnusedAttachmentRef[nSubpassIdx].data();

		}
	}

	std::vector<VkSubpassDependency> vSubpassDependencies;
	for (size_t nSubpassIdx = 1; nSubpassIdx < vSubpass.size(); ++nSubpassIdx)
	{
		for (size_t nDependantIdx = 0; nDependantIdx < nSubpassIdx; ++nDependantIdx)
		{
			vSubpassDependencies.push_back({});
			VkSubpassDependency& depend = vSubpassDependencies.back();
			depend.srcSubpass = nDependantIdx;
			depend.dstSubpass = nSubpassIdx;
			//          depend.dstSubpass = (subpass<subpassCount) ? subpass : VK_SUBPASS_EXTERNAL;
			depend.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			depend.dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			depend.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			depend.dependencyFlags = 0;
		}
	}


	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = vAttachmentDesc.size();
	renderPassInfo.pAttachments = vAttachmentDesc.data();
	renderPassInfo.subpassCount = vSubpass.size();
	renderPassInfo.pSubpasses = vSubpass.data();
	renderPassInfo.dependencyCount = vSubpassDependencies.size();
	renderPassInfo.pDependencies = vSubpassDependencies.data();


	VkRenderPassMultiviewCreateInfo renderPassMultiviewInfo{};//Beware of the lifecycle of variables
	if (pRenderPass->GetViewportNum() > 1)
	{
		renderPassMultiviewInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO;
		renderPassMultiviewInfo.subpassCount = vSubpass.size();
		renderPassMultiviewInfo.pViewMasks = vViewMask.data();
		renderPassMultiviewInfo.correlationMaskCount = vSubpass.size();
		renderPassMultiviewInfo.pCorrelationMasks = vCorrelationMask.data();
		renderPassMultiviewInfo.pNext = nullptr;

		renderPassInfo.pNext = &renderPassMultiviewInfo;
	}

	if (vkCreateRenderPass(m_VkDevice, &renderPassInfo, nullptr, &pRenderPass->m_VkRenderPass) != VK_SUCCESS)
	{
		return false;
	}

	m_PipelineManager.RegisterRenderPass(pRenderPass);

#ifdef MORTY_DEBUG
	SetDebugName(reinterpret_cast<uint64_t>(pRenderPass->m_VkRenderPass), VkObjectType::VK_OBJECT_TYPE_RENDER_PASS, pRenderPass->m_strDebugName.c_str());
#endif

	return true;
}

void MVulkanDevice::DestroyRenderPass(MRenderPass* pRenderPass)
{
	if (pRenderPass)
	{
		if (VK_NULL_HANDLE != pRenderPass->m_VkRenderPass)
		{
			GetRecycleBin()->DestroyRenderPassLater(pRenderPass->m_VkRenderPass);
			pRenderPass->m_VkRenderPass = VK_NULL_HANDLE;
		}

		m_PipelineManager.UnRegisterRenderPass(pRenderPass);
	}
}

bool MVulkanDevice::GenerateFrameBuffer(MRenderPass* pRenderPass)
{
	int fFrameBufferWidth = 0, fFrameBufferHeight = 0;

	std::vector<VkImageView> vAttachmentViews;

	uint32_t unBackNum = pRenderPass->m_vBackTextures.size();
	for (uint32_t backIdx = 0; backIdx < unBackNum; ++backIdx)
	{
		MRenderTarget& backTexture = pRenderPass->m_vBackTextures[backIdx];
		if (!backTexture.pTexture)
		{
			GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateFrameBuffer error: bt == nullptr");
			return false;
		}

		int nWidth = 0, nHeight = 0;
		VkImageView imageView = VK_NULL_HANDLE;

		if (!backTexture.pTexture->GetMipmapsEnable())
		{
			nWidth = backTexture.pTexture->GetSize().x;
			nHeight = backTexture.pTexture->GetSize().y;
			imageView = backTexture.pTexture->m_VkImageView;
		}
		else
		{
			if (backTexture.m_VkImageView == VK_NULL_HANDLE)
			{
				std::shared_ptr<MTexture> pTexture = backTexture.pTexture;
				backTexture.m_VkImageView = CreateImageView(pTexture->m_VkTextureImage, pTexture->m_VkTextureFormat, GetAspectFlags(pTexture.get()), backTexture.desc.nMipmapLevel, 1, GetLayerCount(pTexture.get()), GetImageViewType(pTexture.get()));
				nWidth = pTexture->GetMipmapSize(backTexture.desc.nMipmapLevel).x;
				nHeight = pTexture->GetMipmapSize(backTexture.desc.nMipmapLevel).y;
			}
			imageView = backTexture.m_VkImageView;
		}

		if (nWidth != fFrameBufferWidth || nHeight != fFrameBufferHeight)
		{
			if (fFrameBufferWidth == 0 && fFrameBufferHeight == 0)
			{
				fFrameBufferWidth = nWidth;
				fFrameBufferHeight = nHeight;
			}
			else
			{
				GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateFrameBuffer error: different size");
				return false;
			}
		}

		vAttachmentViews.push_back(imageView);
	}

	std::shared_ptr<MTexture> pDepthTexture = pRenderPass->GetDepthTexture();

	if (pDepthTexture)
	{
		if (pDepthTexture->GetSize().x != fFrameBufferWidth || pDepthTexture->GetSize().y != fFrameBufferHeight)
		{
			if (fFrameBufferWidth == 0 && fFrameBufferHeight == 0)
			{
				fFrameBufferWidth = pDepthTexture->GetSize().x;
				fFrameBufferHeight = pDepthTexture->GetSize().y;
			}
			else
			{
				GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateFrameBuffer error: different size");
				return false;
			}
		}

		if (!pDepthTexture->m_VkImageView)
		{
			pDepthTexture->GenerateBuffer(this);
		}

		vAttachmentViews.push_back(pDepthTexture->m_VkImageView);
	}

	fFrameBufferWidth = std::max(fFrameBufferWidth, 1);
	fFrameBufferHeight = std::max(fFrameBufferHeight, 1);


	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//This renderpass only used to match format.
	framebufferInfo.renderPass = pRenderPass->m_VkRenderPass;

	framebufferInfo.attachmentCount = vAttachmentViews.size();
	framebufferInfo.pAttachments = vAttachmentViews.data();
	framebufferInfo.width = fFrameBufferWidth;
	framebufferInfo.height = fFrameBufferHeight;
	framebufferInfo.layers = 1;


	pRenderPass->m_vkExtent2D.width = fFrameBufferWidth;
	pRenderPass->m_vkExtent2D.height = fFrameBufferHeight;

	VkResult result = vkCreateFramebuffer(m_VkDevice, &framebufferInfo, nullptr, &pRenderPass->m_VkFrameBuffer);
	if (VK_SUCCESS != result)
	{
		GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateFrameBuffer error: vulkan result: {}", std::to_string(result).c_str());
	}

	return true;
}

void MVulkanDevice::DestroyFrameBuffer(MRenderPass* pRenderPass)
{
	if (!pRenderPass)
		return;

	if (pRenderPass->m_VkFrameBuffer)
	{
		GetRecycleBin()->DestroyFramebufferLater(pRenderPass->m_VkFrameBuffer);
		pRenderPass->m_VkFrameBuffer = VK_NULL_HANDLE;
	}

	for (MRenderTarget& backTexture : pRenderPass->m_vBackTextures)
	{
		if (backTexture.m_VkImageView)
		{
			GetRecycleBin()->DestroyImageViewLater(backTexture.m_VkImageView);
			backTexture.m_VkImageView = VK_NULL_HANDLE;
		}
	}
}

bool MVulkanDevice::RegisterComputeDispatcher(MComputeDispatcher* pComputeDispatcher)
{
	return m_PipelineManager.RegisterComputeDispatcher(pComputeDispatcher);
}

bool MVulkanDevice::UnRegisterComputeDispatcher(MComputeDispatcher* pComputeDispatcher)
{
	return m_PipelineManager.UnRegisterComputeDispatcher(pComputeDispatcher);
}

MIRenderCommand* MVulkanDevice::CreateRenderCommand(const MString& strCommandName)
{
	MVulkanPrimaryRenderCommand* pCommand = new MVulkanPrimaryRenderCommand();
	pCommand->m_pDevice = this;

	//New CommandBuffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_VkGraphCommandPool;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(m_VkDevice, &allocInfo, &pCommand->m_VkCommandBuffer);

	SetDebugName((uint64_t)pCommand->m_VkCommandBuffer, VkObjectType::VK_OBJECT_TYPE_COMMAND_BUFFER, strCommandName.c_str());

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	vkCreateFence(m_VkDevice, &fenceInfo, nullptr, &pCommand->m_VkRenderFinishedFence);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCreateSemaphore(m_VkDevice, &semaphoreInfo, nullptr, &pCommand->m_VkRenderFinishedSemaphore);

	m_tFrameData[m_unFrameCount].vCommand.push_back(pCommand);

	return pCommand;
}

void MVulkanDevice::RecoveryRenderCommand(MIRenderCommand* pRenderCommand)
{
	MVulkanPrimaryRenderCommand* pCommand = dynamic_cast<MVulkanPrimaryRenderCommand*>(pRenderCommand);

	if (!pCommand)
		return;

	while (vkGetFenceStatus(m_VkDevice, pCommand->m_VkRenderFinishedFence) != VK_SUCCESS);

	if (pCommand->m_VkCommandBuffer)
	{
		//GetRecycleBin()->DestroyCommandBufferLater(pCommand->m_VkCommandBuffer);
		
		vkFreeCommandBuffers(m_VkDevice, m_VkGraphCommandPool, 1, &pCommand->m_VkCommandBuffer);
		pCommand->m_VkCommandBuffer = VK_NULL_HANDLE;
	}

	if (pCommand->m_VkRenderFinishedFence)
	{
		//GetRecycleBin()->DestroyFenceLater(pCommand->m_VkRenderFinishedFence);
		
		vkDestroyFence(m_VkDevice, pCommand->m_VkRenderFinishedFence, nullptr);
		pCommand->m_VkRenderFinishedFence = VK_NULL_HANDLE;
	}

	if (pCommand->m_VkRenderFinishedSemaphore)
	{
		//GetRecycleBin()->DestroySemaphoreLater(pCommand->m_VkRenderFinishedSemaphore);
		
		vkDestroySemaphore(m_VkDevice, pCommand->m_VkRenderFinishedSemaphore, nullptr);
		pCommand->m_VkRenderFinishedSemaphore = VK_NULL_HANDLE;
	}

	for (MVulkanSecondaryRenderCommand* pSecondaryCommand : pCommand->m_vSecondaryCommand)
	{
		vkFreeCommandBuffers(m_VkDevice, m_VkGraphCommandPool, 1, &pSecondaryCommand->m_VkCommandBuffer);
		pSecondaryCommand->m_VkCommandBuffer = VK_NULL_HANDLE;
	}

	pCommand->m_vSecondaryCommand.clear();

	//TODO Recovery
	delete pCommand;
}

bool MVulkanDevice::IsFinishedCommand(MIRenderCommand* pCommand)
{
	if (MVulkanPrimaryRenderCommand* pVulkanCommand = static_cast<MVulkanPrimaryRenderCommand*>(pCommand))
	{
		return VK_SUCCESS == vkGetFenceStatus(m_VkDevice, pVulkanCommand->m_VkRenderFinishedFence);
	}

	return false;
}

void MVulkanDevice::SubmitCommand(MIRenderCommand* pCommand)
{
	MVulkanPrimaryRenderCommand* pRenderCommand = dynamic_cast<MVulkanPrimaryRenderCommand*>(pCommand);
	if (!pRenderCommand)
		return;

	//submit command
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		std::vector<VkSemaphore>& vWaitSemaphoreBeforeSubmit = pRenderCommand->m_vRenderWaitSemaphore;

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = vWaitSemaphoreBeforeSubmit.size();
		submitInfo.pWaitSemaphores = vWaitSemaphoreBeforeSubmit.data();
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		VkCommandBuffer commandBuffers[] = { pRenderCommand->m_VkCommandBuffer };
		//TODO maybe mutil command buffers for every frame
		submitInfo.pCommandBuffers = commandBuffers;

		VkSemaphore signalSemaphores[] = { pRenderCommand->m_VkRenderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VkFence vkInFightFence = pRenderCommand->m_VkRenderFinishedFence;
		//m_VkInFlightFences = unsigned
		vkResetFences(m_VkDevice, 1, &vkInFightFence);
		if (vkQueueSubmit(m_VkGraphicsQueue, 1, &submitInfo, vkInFightFence) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}
	}
}

void MVulkanDevice::Update()
{
	MIDevice::Update();

	CheckFrameFinish();



	if (m_pDefaultRecycleBin)
	{
		m_pDefaultRecycleBin->EmptyTrash();
	}

	++m_unFrameCount;

	m_tFrameData[m_unFrameCount] = {};
	m_tFrameData[m_unFrameCount].pRecycleBin = m_pRecycleBin = new MVulkanObjectRecycleBin(this);
	m_pRecycleBin->Initialize();
}

void MVulkanDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkBufferCopy region)
{
	VkCommandBuffer commandBuffer = BeginCommands();

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &region);

	EndCommands(commandBuffer);
}

void MVulkanDevice::CopyImageBuffer(VkBuffer srcBuffer, VkImage image, const uint32_t& width, const uint32_t& height, const uint32_t& unCount)
{
	VkCommandBuffer commandBuffer = BeginCommands();


	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = unCount;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, srcBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


	EndCommands(commandBuffer);
}

void MVulkanDevice::CopyImageBuffer(MTexture* pSource, MTexture* pDestination, VkCommandBuffer buffer/* = VK_NULL_HANDLE*/)
{
	VkCommandBuffer commandBuffer = buffer;
	if (VK_NULL_HANDLE == commandBuffer)
	{
		commandBuffer = BeginCommands();
	}

	VkImageSubresourceRange range = {};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseArrayLayer = 0;
	range.layerCount = 1;
	range.levelCount = 1;
	range.baseMipLevel = 0;

	TransitionImageLayout(commandBuffer, pSource->m_VkTextureImage, pSource->m_VkImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range);
	TransitionImageLayout(commandBuffer, pDestination->m_VkTextureImage, pDestination->m_VkImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range);

	VkImageBlit blit{};
	blit.srcOffsets[0] = { 0, 0, 0 };
	blit.srcOffsets[1] = { static_cast<int32_t>(pSource->GetSize().x), static_cast<int32_t>(pSource->GetSize().y), 1 };
	blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.layerCount = 1;
	blit.dstOffsets[0] = { 0, 0, 0 };
	blit.dstOffsets[1] = { static_cast<int32_t>(pDestination->GetSize().x), static_cast<int32_t>(pDestination->GetSize().y), 1 };
	blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.dstSubresource.mipLevel = 0;
	blit.dstSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount = 1;

	vkCmdBlitImage(commandBuffer, pSource->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pDestination->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);


	TransitionImageLayout(commandBuffer, pSource->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pSource->m_VkImageLayout, range);
	TransitionImageLayout(commandBuffer, pDestination->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, pDestination->m_VkImageLayout, range);


	if (VK_NULL_HANDLE == buffer)
	{
		EndCommands(commandBuffer);
	}
}

void MVulkanDevice::GenerateMipmaps(MTexture* pBuffer, const uint32_t& unMipLevels, VkCommandBuffer buffer/* = VK_NULL_HANDLE*/)
{
	if (!pBuffer || unMipLevels <= 1)
		return;

	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(m_VkPhysicalDevice, pBuffer->m_VkTextureFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = buffer;
	if (VK_NULL_HANDLE == commandBuffer)
	{
		commandBuffer = BeginCommands();
	}

	int32_t mipWidth = pBuffer->GetSize().x;
	int32_t mipHeight = pBuffer->GetSize().y;
	uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(pBuffer->GetSize().x, pBuffer->GetSize().y)))) + 1;
	uint32_t unLayerCount = GetLayerCount(pBuffer);

	VkImageSubresourceRange vkSubresourceRange = {};
	vkSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkSubresourceRange.baseMipLevel = 0;
	vkSubresourceRange.levelCount = unMipLevels;
	vkSubresourceRange.layerCount = unLayerCount;
	TransitionImageLayout(commandBuffer, pBuffer->m_VkTextureImage, pBuffer->m_VkImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkSubresourceRange);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = pBuffer->m_VkTextureImage;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = unLayerCount;
	barrier.subresourceRange.levelCount = 1;

	for (uint32_t i = 1; i < mipLevels; ++i) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = unLayerCount;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = unLayerCount;

		vkCmdBlitImage(commandBuffer,
			pBuffer->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			pBuffer->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	if (VK_NULL_HANDLE == buffer)
	{
		EndCommands(commandBuffer);
	}
}

void MVulkanDevice::TransitionImageLayout(VkImage image,VkImageLayout oldLayout,VkImageLayout newLayout, VkImageSubresourceRange subresourceRange)
{
	VkCommandBuffer commandBuffer = BeginCommands();

	TransitionImageLayout(commandBuffer, image, oldLayout, newLayout, subresourceRange);

	EndCommands(commandBuffer);
}

void MVulkanDevice::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange)
{
	VkImageMemoryBarrier imageMemoryBarrier;
	TransitionImageLayout(imageMemoryBarrier, image, oldLayout, newLayout, subresourceRange);

	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

	vkCmdPipelineBarrier(commandBuffer, srcStageFlags, destStageFlags, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &imageMemoryBarrier);
}

void MVulkanDevice::TransitionImageLayout(VkImageMemoryBarrier& imageMemoryBarrier, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange)
{
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = NULL;
	// Some default values
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	switch (oldLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		imageMemoryBarrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

	default:
		MORTY_ASSERT(false);
	}

	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	default:
		MORTY_ASSERT(false);
	}
}

VkImageView MVulkanDevice::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const uint32_t& unMipmap, const uint32_t& unLayerCount, const VkImageViewType& eViewType)
{
	return CreateImageView(image, format, aspectFlags, 0, unMipmap, unLayerCount, eViewType);
}

VkImageView MVulkanDevice::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const uint32_t& unBaseMipmap, const uint32_t& unMipmapCount, const uint32_t& unLayerCount, const VkImageViewType& eViewType)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = eViewType;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = unBaseMipmap;
	viewInfo.subresourceRange.levelCount = unMipmapCount;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = unLayerCount;

	VkImageView imageView;
	if (vkCreateImageView(m_VkDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		VK_NULL_HANDLE;
	}

	return imageView;
}

void MVulkanDevice::CreateImage(const uint32_t& unWidth, const uint32_t& unHeight, const uint32_t& unMipmap, const uint32_t& unLayerCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout defaultLayout, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags createFlag, VkImageType imageType)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = imageType;
	imageInfo.flags = createFlag;
	imageInfo.extent.width = VALUE_MAX(unWidth, 1);
	imageInfo.extent.height = VALUE_MAX(unHeight, 1);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = unMipmap;
	imageInfo.arrayLayers = unLayerCount;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = defaultLayout;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(m_VkDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_VkDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (MGlobal::M_INVALID_UINDEX == allocInfo.memoryTypeIndex)
	{
		MORTY_ASSERT(false);
	}

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(m_VkDevice, image, imageMemory, 0);
}

VkBufferUsageFlags MVulkanDevice::GetBufferUsageFlags(MBuffer* pBuffer) const
{
	if (pBuffer->m_eUsageType == 0)
	{
		MORTY_ASSERT(false);
	}

	VkBufferUsageFlags vkBufferUsageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	if (MBuffer::MUsageType::EVertex & pBuffer->m_eUsageType)
	{
		vkBufferUsageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}
	if (MBuffer::MUsageType::EIndex & pBuffer->m_eUsageType)
	{
		vkBufferUsageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}
	if (MBuffer::MUsageType::EStorage & pBuffer->m_eUsageType)
	{
		vkBufferUsageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}
	if (MBuffer::MUsageType::EUniform & pBuffer->m_eUsageType)
	{
		vkBufferUsageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	if (MBuffer::MUsageType::EIndirect & pBuffer->m_eUsageType)
	{
		vkBufferUsageFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	}


	return vkBufferUsageFlags;
}

VkMemoryPropertyFlags MVulkanDevice::GetMemoryFlags(MBuffer* pBuffer) const
{
	if (MBuffer::MMemoryType::EHostVisible == pBuffer->m_eMemoryType)
	{
		return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	}
	else if (MBuffer::MMemoryType::EDeviceLocal == pBuffer->m_eMemoryType)
	{
		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}
	else
	{
		MORTY_ASSERT(false);
	}
	
	return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
}

bool MVulkanDevice::GenerateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_VkDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		GetEngine()->GetLogger()->Error("vkCreateBuffer failed. file: {}, line: {}", __FILE__, __LINE__);
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_VkDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (MGlobal::M_INVALID_UINDEX == allocInfo.memoryTypeIndex)
	{
		GetEngine()->GetLogger()->Error("memoryTypeIndex invalid. file: {}, line: {}", __FILE__, __LINE__);
		vkDestroyBuffer(m_VkDevice, buffer, nullptr);
		return false;
	}

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		GetEngine()->GetLogger()->Error("vkAllocateMemory failed. file: {}, line: {}", __FILE__, __LINE__);
		vkDestroyBuffer(m_VkDevice, buffer, nullptr);
		return false;
	}

	vkBindBufferMemory(m_VkDevice, buffer, bufferMemory, 0);

	return true;
}

void MVulkanDevice::DestroyBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	vkDestroyBuffer(m_VkDevice, buffer, nullptr);
	vkFreeMemory(m_VkDevice, bufferMemory, nullptr);
}

MVulkanSecondaryRenderCommand* MVulkanDevice::CreateChildCommand(MVulkanPrimaryRenderCommand* pParentCommand)
{
	MORTY_UNUSED(pParentCommand);

	MVulkanSecondaryRenderCommand* pSecondaryCommand = new MVulkanSecondaryRenderCommand();
	pSecondaryCommand->m_pDevice = this;

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandPool = m_VkGraphCommandPool;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(m_VkDevice, &allocInfo, &pSecondaryCommand->m_VkCommandBuffer);

	return pSecondaryCommand;
}

void MVulkanDevice::CheckFrameFinish()
{
	for (auto iter = m_tFrameData.begin(); iter != m_tFrameData.end();)
	{
		auto& vCommand = iter->second.vCommand;
		bool bFinished = true;
		for (MVulkanRenderCommand* pCommand : vCommand)
		{
			pCommand->CheckFinished();
			bFinished &= pCommand->IsFinished();
		}

		if(bFinished)
		{
			for (MVulkanRenderCommand* pCommand : vCommand)
			{
				RecoveryRenderCommand(pCommand);
			}

			if (auto& pRecycleBin = iter->second.pRecycleBin)
			{
				if (m_pRecycleBin == pRecycleBin)
					m_pRecycleBin = nullptr;

				pRecycleBin->Release();
				delete pRecycleBin;
				pRecycleBin = nullptr;
			}

//			GetEngine()->GetLogger()->Information("the Frame Finished: {}", iter->first);
			iter = m_tFrameData.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void MVulkanDevice::WaitFrameFinish()
{
	while (!m_tFrameData.empty())
	{
		CheckFrameFinish();
	}
}

VkCommandBuffer MVulkanDevice::BeginCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_VkGraphCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_VkDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void MVulkanDevice::EndCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_VkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

	VkResult result = vkQueueWaitIdle(m_VkGraphicsQueue);
	MORTY_ASSERT(result == VkResult::VK_SUCCESS);

	vkFreeCommandBuffers(m_VkDevice, m_VkGraphCommandPool, 1, &commandBuffer);
}

bool MVulkanDevice::InitVulkanInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "Morty App";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Morty Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
#ifdef MORTY_MACOS
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
	createInfo.pApplicationInfo = &appInfo;
	
	createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
	createInfo.ppEnabledLayerNames = ValidationLayers.data();


	createInfo.enabledExtensionCount = InstanceExtensions.size();
	createInfo.ppEnabledExtensionNames = InstanceExtensions.data();

	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_VkInstance);

	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
		GetEngine()->GetLogger()->Error(
			"Cannot find a compatible Vulkan installable client "
			"driver (ICD). Please make sure your driver supports "
			"Vulkan before continuing. The call to vkCreateInstance failed.");
		return false;
	}
	else if (result != VK_SUCCESS) {
		GetEngine()->GetLogger()->Error(
			"The call to vkCreateInstance failed. error code: {}", int(result));
		return false;
	}

	vkCmdPushDescriptorSet = reinterpret_cast<PFN_vkCmdPushDescriptorSetKHR>(vkGetInstanceProcAddr(m_VkInstance, "vkCmdPushDescriptorSetKHR"));
	vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(m_VkInstance, "vkSetDebugUtilsObjectNameEXT"));

#if MORTY_DEBUG
	// load kCreateDebugUtilsMessengerEXT
	PFN_vkCreateDebugUtilsMessengerEXT pvkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_VkInstance, "vkCreateDebugUtilsMessengerEXT");
	if (pvkCreateDebugUtilsMessengerEXT == NULL)
		return false;

	// create debug utils messenger
	VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {};
	debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debug_utils_messenger_create_info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	debug_utils_messenger_create_info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debug_utils_messenger_create_info.pfnUserCallback = OutputDebugUtilsMessenger;
	debug_utils_messenger_create_info.pUserData = this;

	if (VK_SUCCESS != pvkCreateDebugUtilsMessengerEXT(m_VkInstance, &debug_utils_messenger_create_info, NULL, &m_VkDebugUtilsMessenger))
		return false;
#endif

	return true;
}

bool MVulkanDevice::InitPhysicalDevice()
{
	uint32_t nDeviceCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(m_VkInstance, &nDeviceCount, NULL);

	if (result != VK_SUCCESS || nDeviceCount < 1)
	{
		GetEngine()->GetLogger()->Error("Initialize Vulkan Error : device count < 1");
		return false;
	}

	std::vector<VkPhysicalDevice> vPhysicalDevices(nDeviceCount);
	result = vkEnumeratePhysicalDevices(m_VkInstance, &nDeviceCount, vPhysicalDevices.data());
	if (result != VK_SUCCESS)
	{
		GetEngine()->GetLogger()->Error("Initialize Vulkan Error : vkEnumeratePhysicalDevices error.");
		return false;
	}

	m_VkPhysicalDeviceProperties = {};

	for (uint32_t i = 0; i < nDeviceCount; i++)
	{
		vkGetPhysicalDeviceProperties(vPhysicalDevices[i], &m_VkPhysicalDeviceProperties);

		if (IsDeviceSuitable(vPhysicalDevices[i]))
		{
			m_VkPhysicalDevice = vPhysicalDevices[i];
			break;
		}
	}

	if (m_VkPhysicalDevice == VK_NULL_HANDLE)
	{
		MORTY_ASSERT(m_VkPhysicalDevice);
		GetEngine()->GetLogger()->Error("Initialize Vulkan Error : m_nPhysicalDeviceIndex == -1");
		return false;
	}

	m_nVulkanVersionMajor = VK_VERSION_MAJOR(m_VkPhysicalDeviceProperties.apiVersion);
	m_nVulkanVersionMinor = VK_VERSION_MINOR(m_VkPhysicalDeviceProperties.apiVersion);
	m_nVulkanVersionPatch = VK_VERSION_PATCH(m_VkPhysicalDeviceProperties.apiVersion);

	GetEngine()->GetLogger()->Information("Vulkan API Version:    {}.{}.{}\n",
										  m_nVulkanVersionMajor,
										  m_nVulkanVersionMinor,
										  m_nVulkanVersionPatch);

	m_nGraphicsFamilyIndex = FindQueueGraphicsFamilies(m_VkPhysicalDevice);
	m_nComputeFamilyIndex = FindQueueComputeFamilies(m_VkPhysicalDevice);

    return true;
}

bool MVulkanDevice::InitLogicalDevice()
{

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

	float priorities[] = { 1.0f };	//range 0~1
	VkDeviceQueueCreateInfo queueInfo{};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = NULL;
	queueInfo.flags = 0;
	queueInfo.queueFamilyIndex = m_nGraphicsFamilyIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = priorities;
	queueCreateInfos.push_back(queueInfo);

	if (m_nComputeFamilyIndex != m_nGraphicsFamilyIndex)
	{
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = m_nComputeFamilyIndex;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = priorities;
		queueCreateInfos.push_back(queueInfo);
	}

	vkGetPhysicalDeviceFeatures(m_VkPhysicalDevice, &m_VkPhysicalDeviceFeatures);

	VkPhysicalDeviceVulkan11Features device11Features = {};
	device11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	device11Features.multiview = VK_TRUE;

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = queueCreateInfos.size();
	deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceInfo.enabledExtensionCount = DeviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = DeviceExtensions.data();
	deviceInfo.pEnabledFeatures = &m_VkPhysicalDeviceFeatures;
	deviceInfo.pNext = &device11Features;


	VkResult result = vkCreateDevice(m_VkPhysicalDevice, &deviceInfo, NULL, &m_VkDevice);
	if (result != VK_SUCCESS)
	{
		GetEngine()->GetLogger()->Error("Initialize Vulkan Error : vkCreateDevice error.");
		return false;
	}

	vkGetDeviceQueue(m_VkDevice, m_nGraphicsFamilyIndex, 0, &m_VkGraphicsQueue);

	return true;
}

bool MVulkanDevice::InitCommandPool()
{

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = m_nGraphicsFamilyIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

	if (vkCreateCommandPool(m_VkDevice, &poolInfo, nullptr, &m_VkGraphCommandPool) != VK_SUCCESS)
		return false;


	return true;
}

bool MVulkanDevice::InitDefaultTexture()
{
	m_ShaderDefaultTexture = std::make_shared<MTexture>();

	m_ShaderDefaultTexture->SetName("Shader Default Texture");
	m_ShaderDefaultTexture->SetMipmapsEnable(false);
	m_ShaderDefaultTexture->SetReadable(false);
	m_ShaderDefaultTexture->SetRenderUsage(METextureRenderUsage::EUnknow);
	m_ShaderDefaultTexture->SetShaderUsage(METextureShaderUsage::ESampler);
	m_ShaderDefaultTexture->SetTextureLayout(METextureLayout::ERGBA_UNORM_8);
	m_ShaderDefaultTexture->SetSize(Vector2(1, 1));

	MByte bytes[4];
	for (size_t i = 0; i < 4; i += 4)
	{
		bytes[i + 0] = 255;
		bytes[i + 1] = 255;
		bytes[i + 2] = 255;
		bytes[i + 3] = 255;
	}
	m_ShaderDefaultTexture->GenerateBuffer(this, bytes);

	m_ShaderDefaultTextureCube = std::make_shared<MTexture>();

	m_ShaderDefaultTextureCube->SetName("Shader Default Texture Cube");
	m_ShaderDefaultTextureCube->SetMipmapsEnable(false);
	m_ShaderDefaultTextureCube->SetReadable(false);
	m_ShaderDefaultTextureCube->SetRenderUsage(METextureRenderUsage::EUnknow);
	m_ShaderDefaultTextureCube->SetShaderUsage(METextureShaderUsage::ESampler);
	m_ShaderDefaultTextureCube->SetTextureLayout(METextureLayout::ERGBA_UNORM_8);
	m_ShaderDefaultTextureCube->SetTextureType(METextureType::ETextureCube);
	m_ShaderDefaultTextureCube->SetImageLayerNum(6);
	m_ShaderDefaultTextureCube->SetSize(Vector2(1, 1));

	MByte cubeBytes[24];
	for (size_t i = 0; i < 24; i += 4)
	{
		cubeBytes[i + 0] = 255;
		cubeBytes[i + 1] = 255;
		cubeBytes[i + 2] = 255;
		cubeBytes[i + 3] = 255;
	}
	m_ShaderDefaultTextureCube->GenerateBuffer(this, cubeBytes);

	m_ShaderDefaultTextureArray = std::make_shared<MTexture>();

	m_ShaderDefaultTextureArray->SetName("Shader Default Texture Array");
	m_ShaderDefaultTextureArray->SetMipmapsEnable(false);
	m_ShaderDefaultTextureArray->SetReadable(false);
	m_ShaderDefaultTextureArray->SetRenderUsage(METextureRenderUsage::EUnknow);
	m_ShaderDefaultTextureArray->SetShaderUsage(METextureShaderUsage::ESampler);
	m_ShaderDefaultTextureArray->SetTextureLayout(METextureLayout::ERGBA_UNORM_8);
	m_ShaderDefaultTextureArray->SetTextureType(METextureType::ETexture2DArray);
	m_ShaderDefaultTextureArray->SetImageLayerNum(1);
	m_ShaderDefaultTextureArray->SetSize(Vector2(1, 1));

	MByte arrayBytes[4];
	for (size_t i = 0; i < 4; i += 4)
	{
		arrayBytes[i + 0] = 255;
		arrayBytes[i + 1] = 255;
		arrayBytes[i + 2] = 255;
		arrayBytes[i + 3] = 255;
	}
	m_ShaderDefaultTextureArray->GenerateBuffer(this, arrayBytes);

	return true;
}

bool MVulkanDevice::IsDeviceSuitable(VkPhysicalDevice device)
{
	if (-1 == FindQueueGraphicsFamilies(device))
		return false;

	if (-1 == FindQueueComputeFamilies(device))
		return false;

	if (!CheckDeviceExtensionSupport(device))
		return false;

	return true;
}

int MVulkanDevice::FindQueueGraphicsFamilies(VkPhysicalDevice device)
{
	int graphicsFamily = -1;

	uint32_t unQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &unQueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> vQueueProperties(unQueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &unQueueFamilyCount, vQueueProperties.data());

	for (size_t i = 0; i < unQueueFamilyCount; ++i)
	{
		if (vQueueProperties[i].queueCount > 0 && vQueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			graphicsFamily = i;

		if (graphicsFamily >= 0)
			break;
	}

	return graphicsFamily;
}

int MVulkanDevice::FindQueuePresentFamilies(VkSurfaceKHR surface)
{

	int nPresentFamily = -1;

	uint32_t unQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_VkPhysicalDevice, &unQueueFamilyCount, NULL);

	std::vector<VkQueueFamilyProperties> vQueueProperties(unQueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_VkPhysicalDevice, &unQueueFamilyCount, vQueueProperties.data());

	VkBool32 bSupportsPresenting(VK_FALSE);
	for (uint32_t i = 0; i < unQueueFamilyCount; ++i)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(m_VkPhysicalDevice, i, surface, &bSupportsPresenting);

		if (vQueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (bSupportsPresenting == VK_TRUE) {
				nPresentFamily = i;
				break;
			}
		}
	}

	return nPresentFamily;
}

int MVulkanDevice::FindQueueComputeFamilies(VkPhysicalDevice device)
{
	int graphicsFamily = -1;

	uint32_t unQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &unQueueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> vQueueProperties(unQueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &unQueueFamilyCount, vQueueProperties.data());

	for (size_t i = 0; i < unQueueFamilyCount; ++i)
	{
		if (vQueueProperties[i].queueCount > 0 && vQueueProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			graphicsFamily = i;

		if (graphicsFamily >= 0)
			break;
	}

	return graphicsFamily;
}

bool MVulkanDevice::MultiDrawIndirectSupport() const
{
    return m_VkPhysicalDeviceFeatures.multiDrawIndirect;
}

bool MVulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	for (const auto& extension : availableExtensions) {

		// GetEngine()->GetLogger()->Information("Support extension: {}.", extension.extensionName);

		requiredExtensions.erase(extension.extensionName);
	}

	for (auto extensionName : requiredExtensions)
	{
		GetEngine()->GetLogger()->Error("Not support extension: {}.", extensionName.c_str());
	}

	return requiredExtensions.empty();
}

#endif
