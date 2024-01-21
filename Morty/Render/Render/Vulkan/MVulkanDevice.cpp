#include "Render/Vulkan/MVulkanDevice.h"

#include "MVulkanPhysicalDevice.h"
#include "Render/MMesh.h"
#include "Render/MBuffer.h"
#include "Engine/MEngine.h"
#include "Basic/MTexture.h"
#include "Resource/MResource.h"
#include "Utility/MFileHelper.h"
#include "Shader/MShaderParam.h"
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

MVulkanDevice::MVulkanDevice()
	: MIDevice()
	, m_ShaderCompiler(this)
    , m_ShaderReflector(this)
	, m_PipelineManager(this)
	, m_BufferPool(this)
{

}

bool MVulkanDevice::Initialize()
{
	m_pPhysicalDevice = std::make_unique<MVulkanPhysicalDevice>(GetEngine());
	m_pPhysicalDevice->Initialize();

	if (!InitLogicalDevice())
		return false;

	if (!InitCommandPool())
		return false;

	if (!InitDescriptorPool())
		return false;

	CreateRecycleBin();

	if (!m_BufferPool.Initialize())
		return false;

	InitSampler();

	m_ShaderReflector.Initialize();


	return true;
}

void MVulkanDevice::Release()
{
	WaitFrameFinish();

	m_pRecycleBin = new MVulkanObjectRecycleBin(this);
	m_pRecycleBin->Initialize();

	m_PipelineManager.Release();
	m_BufferPool.Release();

	for (auto pr : m_tFrameData)
	{
		pr.second.pRecycleBin->Release();
		delete pr.second.pRecycleBin;
		pr.second.pRecycleBin = nullptr;
	}
	m_tFrameData.clear();
	m_pRecycleBin->Release();
	MORTY_SAFE_DELETE(m_pRecycleBin);

	vkDestroySampler(m_VkDevice, m_VkLinearSampler, nullptr);
	vkDestroySampler(m_VkDevice, m_VkNearestSampler, nullptr);
	vkDestroyDescriptorPool(m_VkDevice, m_VkDescriptorPool, nullptr);
	vkDestroyCommandPool(m_VkDevice, m_VkPresetCommandPool, nullptr);
	vkDestroyCommandPool(m_VkDevice, m_VkTemporaryCommandPool, nullptr);
	vkDestroyDevice(m_VkDevice, nullptr);

	m_pPhysicalDevice->Release();
	m_pPhysicalDevice = nullptr;
}

VkFormat MVulkanDevice::GetFormat(const METextureLayout& layout) const
{
	static const std::unordered_map<METextureLayout, VkFormat> FormatTable = {
		{METextureLayout::ER_UNORM_8, VK_FORMAT_R8_UNORM},
		{METextureLayout::ERG_UNORM_8, VK_FORMAT_R8G8_UNORM},
		{METextureLayout::ERGB_UNORM_8, VK_FORMAT_R8G8B8_UNORM},
		{METextureLayout::ERGBA_UNORM_8, VK_FORMAT_R8G8B8A8_UNORM},
		{METextureLayout::ER_UINT_8, VK_FORMAT_R8_UINT},
		{METextureLayout::ER_FLOAT_16, VK_FORMAT_R16_SFLOAT},
		{METextureLayout::ERG_FLOAT_16, VK_FORMAT_R16G16_SFLOAT},
		{METextureLayout::ERGB_FLOAT_16, VK_FORMAT_R16G16B16_SFLOAT},
		{METextureLayout::ERGBA_FLOAT_16, VK_FORMAT_R16G16B16A16_SFLOAT},
		{METextureLayout::ER_FLOAT_32, VK_FORMAT_R32_SFLOAT},
		{METextureLayout::ERG_FLOAT_32, VK_FORMAT_R32G32_SFLOAT},
		{METextureLayout::ERGB_FLOAT_32, VK_FORMAT_R32G32B32_SFLOAT},
		{METextureLayout::ERGBA_FLOAT_32, VK_FORMAT_R32G32B32A32_SFLOAT},

	};

	if (METextureLayout::EDepth == layout)
	{
		return m_pPhysicalDevice->m_VkDepthTextureFormat;
	}

	const auto findResult = FormatTable.find(layout);
	if (findResult != FormatTable.end())
	{
		return findResult->second;
	}

	return VK_FORMAT_R8G8B8A8_SRGB;
}

VkImageUsageFlags MVulkanDevice::GetUsageFlags(MTexture* pTexture) const
{
	VkImageUsageFlags usageFlags = 0;
	if (pTexture->GetShaderUsage() & METextureReadUsage::EPixelSampler)
	{
		usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (pTexture->GetShaderUsage() & METextureReadUsage::EStorageRead)
	{
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if (pTexture->GetShaderUsage() & METextureReadUsage::EShadingRateMask)
	{
		usageFlags |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
	}

	if (pTexture->GetRenderUsage() == METextureWriteUsage::EStorageWrite)
	{
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	else if (pTexture->GetRenderUsage() == METextureWriteUsage::ERenderBack)
	{
		usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	else if (pTexture->GetRenderUsage() == METextureWriteUsage::ERenderPresent)
	{
		usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			| VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	else if (pTexture->GetRenderUsage() == METextureWriteUsage::ERenderDepth)
	{
		usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else if (pTexture->GetRenderUsage() == METextureWriteUsage::EUnknow)
	{
		//nothing.
	}
    else
    {
		MORTY_ASSERT(false);
    }

	if (pTexture->GetReadable())
	{
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	return usageFlags;
}

VkImageAspectFlags MVulkanDevice::GetAspectFlags(METextureWriteUsage eUsage) const
{
	if (eUsage == METextureWriteUsage::ERenderDepth)
	{
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	return VK_IMAGE_ASPECT_COLOR_BIT;
}
VkImageAspectFlags MVulkanDevice::GetAspectFlags(VkFormat format) const
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

VkImageAspectFlags MVulkanDevice::GetAspectFlags(VkImageLayout layout) const
{
	static const std::unordered_map<VkImageLayout, VkImageAspectFlags> LayoutToAspect = {
		{VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_MEMORY_WRITE_BIT},
		{VK_IMAGE_LAYOUT_UNDEFINED, 0},
		{VK_IMAGE_LAYOUT_PREINITIALIZED, VK_ACCESS_HOST_WRITE_BIT},
		{VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT},
		{VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT},
		{VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT},
		{VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT},
		{VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT},
		{VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT},
		{VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT},
		{VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR, VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR}

	};

	const auto aspectFlag = LayoutToAspect.find(layout);
	if (aspectFlag == LayoutToAspect.end())
	{
		MORTY_ASSERT(false);
		return 0;
	}

	return aspectFlag->second;
}

VkImageLayout MVulkanDevice::GetImageLayout(MTexture* pTexture) const
{
	if (pTexture->GetRenderUsage() == METextureWriteUsage::ERenderPresent)
	{
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	if (pTexture->GetRenderUsage() == METextureWriteUsage::EStorageWrite)
	{
		return VK_IMAGE_LAYOUT_GENERAL;
	}

	if (pTexture->GetRenderUsage() == METextureWriteUsage::ERenderDepth)
	{
		return m_pPhysicalDevice->m_VkDepthImageLayout;
	}
	if (pTexture->GetRenderUsage() == METextureWriteUsage::ERenderBack)
	{
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	if (pTexture->GetShaderUsage() & METextureReadUsage::EPixelSampler)
	{
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	MORTY_ASSERT(false);
	return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkImageViewType MVulkanDevice::GetImageViewType(MTexture* pTexture) const
{
	static const std::unordered_map<METextureType, VkImageViewType> ViewTypeTable = {
		{METextureType::ETextureCube, VK_IMAGE_VIEW_TYPE_CUBE},
		{METextureType::ETexture2DArray, VK_IMAGE_VIEW_TYPE_2D_ARRAY},
		{METextureType::ETexture3D, VK_IMAGE_VIEW_TYPE_3D},
		{METextureType::ETexture2D, VK_IMAGE_VIEW_TYPE_2D},

	};

	const auto findResult = ViewTypeTable.find(pTexture->GetTextureType());
	MORTY_ASSERT(findResult != ViewTypeTable.end());

	if (findResult != ViewTypeTable.end())
	{
		return findResult->second;
	}
		
	return VK_IMAGE_VIEW_TYPE_2D;
}

VkImageCreateFlags MVulkanDevice::GetImageCreateFlags(MTexture* pTexture) const
{
	static const std::unordered_map<METextureType, VkImageCreateFlags> FlagTable = {
		{METextureType::ETextureCube, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT},
		{METextureType::ETexture2DArray, 0},
		{METextureType::ETexture3D, 0},
		{METextureType::ETexture2D, 0},

	};

	const auto findResult = FlagTable.find(pTexture->GetTextureType());
	MORTY_ASSERT(findResult != FlagTable.end());

	if (findResult != FlagTable.end())
	{
		return findResult->second;
	}

	return 0;
}

VkImageType MVulkanDevice::GetImageType(MTexture* pTexture) const
{
	static const std::unordered_map<METextureType, VkImageType> ImageTypeTable = {
	    {METextureType::ETextureCube, VK_IMAGE_TYPE_2D},
	    {METextureType::ETexture2DArray, VK_IMAGE_TYPE_2D},
	    {METextureType::ETexture3D, VK_IMAGE_TYPE_3D},
	    {METextureType::ETexture2D, VK_IMAGE_TYPE_2D},
	};
	const auto findResult = ImageTypeTable.find(pTexture->GetTextureType());
	MORTY_ASSERT(findResult != ImageTypeTable.end());

	if (findResult != ImageTypeTable.end())
	{
		return findResult->second;
	}

	return VK_IMAGE_TYPE_2D;
}

uint32_t MVulkanDevice::GetMipmapCount(MTexture* pTexture) const
{
    uint32_t unMipmap = 1;

	if (pTexture->GetMipmapsEnable())
	{
		unMipmap = static_cast<uint32_t>(std::floor(std::log2(std::max(pTexture->GetSize().x, pTexture->GetSize().y)))) + 1;
	}

	return unMipmap;
}

uint32_t MVulkanDevice::GetLayerCount(MTexture* pTexture) const
{
	if (!pTexture)
	{
		return 0;
	}
	if (pTexture->GetTextureType() == METextureType::ETextureCube)
	{
		return 6;
	}

	return pTexture->GetImageLayerNum();
}

uint32_t MVulkanDevice::GetBufferBarrierQueueFamily(MEBufferBarrierStage stage) const
{
	switch (stage)
	{
	case MEBufferBarrierStage::EComputeShaderWrite: return m_pPhysicalDevice->m_nComputeFamilyIndex;
	case MEBufferBarrierStage::EComputeShaderRead: return m_pPhysicalDevice->m_nComputeFamilyIndex;
	case MEBufferBarrierStage::EPixelShaderWrite: return m_pPhysicalDevice->m_nGraphicsFamilyIndex;
	case MEBufferBarrierStage::EPixelShaderRead: return m_pPhysicalDevice->m_nGraphicsFamilyIndex;
	case MEBufferBarrierStage::EDrawIndirectRead: return m_pPhysicalDevice->m_nGraphicsFamilyIndex;
	case MEBufferBarrierStage::EUnknow:
		MORTY_ASSERT(stage != MEBufferBarrierStage::EUnknow);
		break;
	default:
		MORTY_ASSERT(false);
		break;
	}
	return 0;
}

MVulkanObjectRecycleBin* MVulkanDevice::GetRecycleBin() const
{
    return m_pRecycleBin;
}

void MVulkanDevice::SetDebugName(uint64_t object, const VkObjectType& type, const char* svDebugName) const
{
#if MORTY_DEBUG
	VkDebugUtilsObjectNameInfoEXT vkObjectName;
	vkObjectName.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	vkObjectName.objectType = type;
	vkObjectName.pNext = nullptr;
	vkObjectName.objectHandle = object;
	vkObjectName.pObjectName = svDebugName;
	m_pPhysicalDevice->vkSetDebugUtilsObjectNameEXT(m_VkDevice, &vkObjectName);
#else
	MORTY_UNUSED(object);
	MORTY_UNUSED(type);
	MORTY_UNUSED(svDebugName);
#endif
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

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	if (vkCreateSampler(m_VkDevice, &samplerInfo, nullptr, &m_VkNearestSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	return true;
}

void MVulkanDevice::CreateRecycleBin()
{
	m_tFrameData[m_unFrameCount] = {};
	m_tFrameData[m_unFrameCount].pRecycleBin = m_pRecycleBin = new MVulkanObjectRecycleBin(this);
	m_pRecycleBin->Initialize();
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
		{
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			unSwapChainNum,
		}
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.poolSizeCount = static_cast<uint32_t>(vPoolSize.size());
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

	MORTY_ASSERT(vkDeviceMemory != VK_NULL_HANDLE);
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
	uint32_t depth = std::max(static_cast<int>(pTexture->GetSize().z), 1);
	
    VkFormat format = GetFormat(pTexture->GetTextureLayout());

	VkImageUsageFlags usageFlags = GetUsageFlags(pTexture);
	VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkImageAspectFlags aspectFlgas = GetAspectFlags(pTexture->GetRenderUsage());
	VkImageLayout defaultLayout = UndefinedImageLayout;

	VkImage textureImage = VK_NULL_HANDLE;
	VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
	VkImageCreateFlags createFlags = GetImageCreateFlags(pTexture);
	VkImageType imageType = GetImageType(pTexture);
	auto nMipmapCount = static_cast<uint32_t>(GetMipmapCount(pTexture));
	auto nLayerCount = static_cast<uint32_t>(GetLayerCount(pTexture));

	VkDeviceSize imageSize = static_cast<uint64_t>(MTexture::GetImageMemorySize(pTexture->GetTextureLayout())) * width * height * depth * nLayerCount;

	if (pTexture->GetRenderUsage() == METextureWriteUsage::ERenderPresent)
	{
		VkImageSubresourceRange vkSubresourceRange = {};
		vkSubresourceRange.aspectMask = aspectFlgas;
		vkSubresourceRange.baseMipLevel = 0;
		vkSubresourceRange.levelCount = nMipmapCount;
		vkSubresourceRange.layerCount = nLayerCount;
		TransitionImageLayout(pTexture->m_VkTextureImage, UndefinedImageLayout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vkSubresourceRange);


		pTexture->m_unMipmapLevel = 1;
	}
	else
	{
		if (pTexture->m_VkTextureImage)
		{
			MORTY_ASSERT(false);
		}

		CreateImage(width, height, depth, nMipmapCount, nLayerCount, format, VK_IMAGE_TILING_OPTIMAL, usageFlags, memoryFlags, defaultLayout, textureImage, textureImageMemory, createFlags, imageType);

		if (pData)
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
			vkSubresourceRange.levelCount = nMipmapCount;
			vkSubresourceRange.layerCount = nLayerCount;

			VkCommandBuffer commandBuffer = BeginCommands();
			TransitionImageLayout(commandBuffer, textureImage, UndefinedImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkSubresourceRange);

			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = nLayerCount;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };

			vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			EndCommands(commandBuffer);

			defaultLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			DestroyBuffer(stagingBuffer, stagingBufferMemory);
		}

		VkImageSubresourceRange vkSubresourceRange = {};
		vkSubresourceRange.aspectMask = GetAspectFlags(format);
		vkSubresourceRange.baseMipLevel = 0;
		vkSubresourceRange.levelCount = nMipmapCount;
		vkSubresourceRange.layerCount = nLayerCount;
		TransitionImageLayout(textureImage, defaultLayout, GetImageLayout(pTexture), vkSubresourceRange);
	
		

		pTexture->m_VkTextureImage = textureImage;
		pTexture->m_unMipmapLevel = nMipmapCount;
		pTexture->m_VkTextureImageMemory = textureImageMemory;
		pTexture->m_VkTextureFormat = format;
		pTexture->m_VkImageLayout = GetImageLayout(pTexture);
	}


	pTexture->m_VkImageView = CreateImageView(pTexture->m_VkTextureImage, pTexture->m_VkTextureFormat, aspectFlgas, nMipmapCount, nLayerCount, GetImageViewType(pTexture));
	

	if (nMipmapCount > 1)
	{
		GenerateMipmaps(pTexture, nMipmapCount);
	}

	if (pTexture->GetRenderUsage() == METextureWriteUsage::ERenderDepth)
	{
		VkFilter vkShadowMapFilter = m_pPhysicalDevice->FormatIsFilterable(format, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

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

	if (pTexture->GetRenderUsage() != METextureWriteUsage::ERenderPresent && pTexture->m_VkTextureImage)
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

VkAttachmentDescription2 CreateAttachmentDescriptionFromTexture(const MRenderTarget& renderTarget)
{
	MORTY_ASSERT(renderTarget.pTexture);

	MORTY_ASSERT(renderTarget.pTexture->m_VkTextureImage);

	VkAttachmentDescription2 colorAttachment = {};
	colorAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;

	if (renderTarget.desc.bClearWhenRender)
	{
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	}
	else
	{
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	}

	colorAttachment.format = renderTarget.pTexture->m_VkTextureFormat;

	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	if (renderTarget.pTexture->GetRenderUsage() == METextureWriteUsage::ERenderPresent)
	{
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	else if (renderTarget.pTexture->GetRenderUsage() == METextureWriteUsage::ERenderBack)
	{
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	else if (renderTarget.pTexture->GetRenderUsage() == METextureWriteUsage::ERenderDepth)
	{
		if (renderTarget.pTexture->GetShaderUsage() & METextureReadUsage::EPixelSampler)
		{
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}
	}
	else if (renderTarget.pTexture->GetShaderUsage() & METextureReadUsage::EShadingRateMask)
	{
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
	}
    else
    {
	    MORTY_ASSERT(false);
    }

	if (renderTarget.desc.bClearWhenRender)
	{
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}
	else
	{
		colorAttachment.initialLayout = colorAttachment.finalLayout;
	}




	return colorAttachment;
}

bool MVulkanDevice::GenerateRenderPass(MRenderPass* pRenderPass)
{
	if (!pRenderPass)
		return false;

	if (VK_NULL_HANDLE != pRenderPass->m_VkRenderPass)
	{
		DestroyRenderPass(pRenderPass);
	}

    uint32_t unBackNum = static_cast<uint32_t>(pRenderPass->m_renderTarget.backTargets.size());

	std::vector<VkAttachmentDescription2> vAttachmentDesc;

	for (uint32_t i = 0; i < unBackNum; ++i)
	{
		MRenderTarget& backTexture = pRenderPass->m_renderTarget.backTargets[i];
		auto attachment = CreateAttachmentDescriptionFromTexture(backTexture);
		vAttachmentDesc.push_back(attachment);
	}
	
	if (std::shared_ptr<MTexture> pDepthTexture = pRenderPass->GetDepthTexture())
	{
		auto attachment = CreateAttachmentDescriptionFromTexture(pRenderPass->m_renderTarget.depthTarget);
		vAttachmentDesc.push_back(attachment);
	}

	

	std::vector<VkSubpassDescription2> vSubpass;

	std::vector<std::vector<VkAttachmentReference2>> vOutAttachmentRef;
	std::vector<std::vector<VkAttachmentReference2>> vOutDepthAttachmentRef;
	std::vector<std::vector<VkAttachmentReference2>> vInAttachmentRef;
	std::vector<std::vector<uint32_t>> vUnusedAttachmentRef;

	// correlationMask 
	// describe which views have similar ability.
	// TODO: assume all views here have the same functionality
	std::vector<uint32_t> vCorrelationMask;
	//vCorrelationMask.push_back((1 << (pRenderPass->GetViewportNum()) - 1);


	// m_ShaderDefaultTexture subpass
	if (pRenderPass->m_vSubpass.empty())
	{
		vOutAttachmentRef.resize(1);
		vOutDepthAttachmentRef.resize(1);

		vSubpass.push_back({});
		auto& vkSubpass = vSubpass.back();
		vkSubpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
		vkSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		for (uint32_t i = 0; i < unBackNum; ++i)
		{
			vOutAttachmentRef[0].push_back({ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, uint32_t(vOutAttachmentRef[0].size()), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT });
		}

		vkSubpass.colorAttachmentCount = unBackNum;

		if (pRenderPass->GetDepthTexture())
		{
			vOutDepthAttachmentRef[0] = { { VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, uint32_t(vOutAttachmentRef[0].size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT } };
			vkSubpass.pDepthStencilAttachment = vOutDepthAttachmentRef[0].data();
		}

		vkSubpass.pColorAttachments = vOutAttachmentRef[0].data();
		vkSubpass.viewMask = (1 << pRenderPass->GetViewportNum()) - 1;
	}
	else  //
	{
		uint32_t unSubpassNum = static_cast<uint32_t>(pRenderPass->m_vSubpass.size());
		vOutAttachmentRef.resize(unSubpassNum);
		vOutDepthAttachmentRef.resize(unSubpassNum);
		vInAttachmentRef.resize(unSubpassNum);
		vUnusedAttachmentRef.resize(unSubpassNum);

		for (uint32_t nSubpassIdx = 0; nSubpassIdx < unSubpassNum; ++nSubpassIdx)
		{
			MSubpass& subpass = pRenderPass->m_vSubpass[nSubpassIdx];

			vSubpass.push_back({});
			auto& vkSubpass = vSubpass.back();
			vkSubpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
			vkSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			vkSubpass.viewMask = 0;

			std::set<uint32_t> vUsedAttachIndex;

			for (uint32_t i = 0; i < subpass.m_vOutputIndex.size(); ++i)
			{
				uint32_t nBackIdx = subpass.m_vOutputIndex[i];

				vOutAttachmentRef[nSubpassIdx].push_back({ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr, nBackIdx, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT });

				vUsedAttachIndex.insert(nBackIdx);
			}

			if (pRenderPass->GetDepthTexture())
			{
				vOutDepthAttachmentRef[nSubpassIdx] = { { VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr,uint32_t(vOutAttachmentRef[nSubpassIdx].size()), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT } };
				vkSubpass.pDepthStencilAttachment = vOutDepthAttachmentRef[nSubpassIdx].data();
			}

			vkSubpass.colorAttachmentCount = static_cast<uint32_t>(vOutAttachmentRef[nSubpassIdx].size());
			vkSubpass.pColorAttachments = vOutAttachmentRef[nSubpassIdx].data();

			for (uint32_t i = 0; i < subpass.m_vInputIndex.size(); ++i)
			{
				uint32_t nBackIdx = subpass.m_vInputIndex[i];

				vInAttachmentRef[nSubpassIdx].push_back({ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2, nullptr,nBackIdx, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT });

				vUsedAttachIndex.insert(nBackIdx);
			}

			vkSubpass.inputAttachmentCount = static_cast<uint32_t>(vInAttachmentRef[nSubpassIdx].size());
			vkSubpass.pInputAttachments = vInAttachmentRef[nSubpassIdx].data();

			for (uint32_t i = 0; i < unBackNum; ++i)
			{
				if (vUsedAttachIndex.find(i) == vUsedAttachIndex.end())
				{
					vUnusedAttachmentRef[nSubpassIdx].push_back(i);
				}
			}

			vkSubpass.preserveAttachmentCount = static_cast<uint32_t>(vUnusedAttachmentRef[nSubpassIdx].size());
			vkSubpass.pPreserveAttachments = vUnusedAttachmentRef[nSubpassIdx].data();
		}
	}

	std::vector<VkSubpassDependency2> vSubpassDependencies;
	for (size_t nSubpassIdx = 1; nSubpassIdx < vSubpass.size(); ++nSubpassIdx)
	{
		for (size_t nDependantIdx = 0; nDependantIdx < nSubpassIdx; ++nDependantIdx)
		{
			vSubpassDependencies.push_back({});
			VkSubpassDependency2& depend = vSubpassDependencies.back();
			depend.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
			depend.srcSubpass = static_cast<uint32_t>(nDependantIdx);
			depend.dstSubpass = static_cast<uint32_t>(nSubpassIdx);
			//          depend.dstSubpass = (subpass<subpassCount) ? subpass : VK_SUBPASS_EXTERNAL;
			depend.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			depend.dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			depend.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			depend.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			depend.dependencyFlags = 0;
		}
	}


	VkRenderPassCreateInfo2 renderPassInfo{};

	VkAttachmentReference2 vkFragShadingRateReference = {};
	VkFragmentShadingRateAttachmentInfoKHR vkShadingRateAttachmentInfo = {};

	if (GetDeviceFeatureSupport(MEDeviceFeature::EVariableRateShading))
	{
		if (pRenderPass->GetShadingRateTexture())
		{
			vAttachmentDesc.push_back(CreateAttachmentDescriptionFromTexture(pRenderPass->m_renderTarget.shadingRate));

			vkFragShadingRateReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
			vkFragShadingRateReference.attachment = vAttachmentDesc.size() - 1;
			vkFragShadingRateReference.layout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;

			vkShadingRateAttachmentInfo.sType = VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR;
			vkShadingRateAttachmentInfo.pFragmentShadingRateAttachment = &vkFragShadingRateReference;
			vkShadingRateAttachmentInfo.shadingRateAttachmentTexelSize = GetPhysicalDevice()->m_VkFragmentShadingRateProperties.maxFragmentShadingRateAttachmentTexelSize;

			for (size_t nSubPassIdx = 0; nSubPassIdx < vSubpass.size(); ++nSubPassIdx)
			{
				vSubpass[nSubPassIdx].pNext = &vkShadingRateAttachmentInfo;
			}
		}
	}


	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(vAttachmentDesc.size());
	renderPassInfo.pAttachments = vAttachmentDesc.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(vSubpass.size());
	renderPassInfo.pSubpasses = vSubpass.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(vSubpassDependencies.size());
	renderPassInfo.pDependencies = vSubpassDependencies.data();
	renderPassInfo.correlatedViewMaskCount = vCorrelationMask.size();
	renderPassInfo.pCorrelatedViewMasks = vCorrelationMask.data();

	if (vkCreateRenderPass2(m_VkDevice, &renderPassInfo, nullptr, &pRenderPass->m_VkRenderPass) != VK_SUCCESS)
	{
		return false;
	}

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
	}
}

std::tuple<VkImageView, Vector2i> MVulkanDevice::CreateFrameBufferViewFromRenderTarget(MRenderTarget& renderTarget)
{
	Vector2i i2Size = {0, 0};
	VkImageView imageView = VK_NULL_HANDLE;

	if (!renderTarget.pTexture->GetMipmapsEnable())
	{
		i2Size.x = renderTarget.pTexture->GetSize().x;
		i2Size.y = renderTarget.pTexture->GetSize().y;
		imageView = renderTarget.pTexture->m_VkImageView;
	}
	else
	{
		if (renderTarget.m_VkImageView == VK_NULL_HANDLE)
		{
			std::shared_ptr<MTexture> pTexture = renderTarget.pTexture;
			renderTarget.m_VkImageView = CreateImageView(pTexture->m_VkTextureImage, pTexture->m_VkTextureFormat, GetAspectFlags(pTexture->GetRenderUsage()), renderTarget.desc.nMipmapLevel, 1, GetLayerCount(pTexture.get()), GetImageViewType(pTexture.get()));
			i2Size.x = pTexture->GetMipmapSize(renderTarget.desc.nMipmapLevel).x;
			i2Size.y = pTexture->GetMipmapSize(renderTarget.desc.nMipmapLevel).y;
		}
		imageView = renderTarget.m_VkImageView;
	}


	return { imageView, i2Size };
}

bool MVulkanDevice::GenerateFrameBuffer(MRenderPass* pRenderPass)
{
	std::vector<std::tuple<VkImageView, Vector2i>> vAttachmentViewAndSize;

	const uint32_t unBackNum = static_cast<uint32_t>(pRenderPass->m_renderTarget.backTargets.size());
	for (uint32_t backIdx = 0; backIdx < unBackNum; ++backIdx)
	{
		MRenderTarget& backTexture = pRenderPass->m_renderTarget.backTargets[backIdx];
		if (!backTexture.pTexture)
		{
			GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateFrameBuffer error: bt == nullptr");
			return false;
		}

		vAttachmentViewAndSize.push_back(CreateFrameBufferViewFromRenderTarget(backTexture));
	}

	if (pRenderPass->GetDepthTexture())
	{
		vAttachmentViewAndSize.push_back(CreateFrameBufferViewFromRenderTarget(pRenderPass->m_renderTarget.depthTarget));
	}

	const Vector2i nFrameBufferSize = vAttachmentViewAndSize.empty() ? Vector2i(0, 0) : std::get<1>(vAttachmentViewAndSize[0]);

	for (size_t nIdx = 1; nIdx < vAttachmentViewAndSize.size(); ++nIdx)
	{
		if (nFrameBufferSize != std::get<1>(vAttachmentViewAndSize[nIdx]))
		{
			GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateFrameBuffer error: different size");
			return false;
		}
	}
	
	std::vector<VkImageView> vAttachmentViews(vAttachmentViewAndSize.size());
	std::transform(vAttachmentViewAndSize.begin(), vAttachmentViewAndSize.end(), vAttachmentViews.begin(), [](const auto& tuple)
		{
			return std::get<0>(tuple);
		});

	if (pRenderPass->GetShadingRateTexture())
	{
		const auto viewAndSize = CreateFrameBufferViewFromRenderTarget(pRenderPass->m_renderTarget.shadingRate);
		const Vector2i n2Size = std::get<1>(viewAndSize);
		const Vector2i vkTexelSize = GetShadingRateTextureTexelSize();
		const Vector2i n2CompareSize = { n2Size.x * vkTexelSize.x, n2Size.y * vkTexelSize.y };

		if(n2CompareSize.x < nFrameBufferSize.x || n2CompareSize.y < nFrameBufferSize.y)
		{
			GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateFrameBuffer error: shading rate texture size not match: ({}, {}), attachment texel size is ({}, {}), frame buffer size is ({}, {})"
				, n2Size.x, n2Size.y, vkTexelSize.x, vkTexelSize.y, nFrameBufferSize.x, nFrameBufferSize.y);

			return false;
		}

		vAttachmentViews.push_back(std::get<0>(viewAndSize));
	}


	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//This renderpass only used to match format.
	framebufferInfo.renderPass = pRenderPass->m_VkRenderPass;

	framebufferInfo.attachmentCount = static_cast<uint32_t>(vAttachmentViews.size());
	framebufferInfo.pAttachments = vAttachmentViews.data();
	framebufferInfo.width = std::max(nFrameBufferSize.x, 1);
	framebufferInfo.height = std::max(nFrameBufferSize.y, 1);
	framebufferInfo.layers = 1;

	pRenderPass->m_vkExtent2D.width = framebufferInfo.width;
	pRenderPass->m_vkExtent2D.height = framebufferInfo.height;

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

	for (MRenderTarget& backTexture : pRenderPass->m_renderTarget.backTargets)
	{
		if (backTexture.m_VkImageView)
		{
			GetRecycleBin()->DestroyImageViewLater(backTexture.m_VkImageView);
			backTexture.m_VkImageView = VK_NULL_HANDLE;
		}
	}
}

MIRenderCommand* MVulkanDevice::CreateRenderCommand(const MString& strCommandName)
{
	MVulkanPrimaryRenderCommand* pCommand = new MVulkanPrimaryRenderCommand();
	pCommand->m_pDevice = this;

	//New CommandBuffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_VkPresetCommandPool;
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
		
		vkFreeCommandBuffers(m_VkDevice, m_VkPresetCommandPool, 1, &pCommand->m_VkCommandBuffer);
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
		vkFreeCommandBuffers(m_VkDevice, m_VkPresetCommandPool, 1, &pSecondaryCommand->m_VkCommandBuffer);
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
	auto funcSubmitWork = [=]() {
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		std::vector<VkSemaphore>& vWaitSemaphoreBeforeSubmit = pRenderCommand->m_vRenderWaitSemaphore;

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(vWaitSemaphoreBeforeSubmit.size());
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
		if (vkQueueSubmit(m_VkPresetQueue, 1, &submitInfo, vkInFightFence) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		pRenderCommand->MarkFinished();
	};


	MThreadWork submitWork;
	submitWork.funcWorkFunction = funcSubmitWork;
	submitWork.eThreadType = MRenderGlobal::THREAD_ID_SUBMIT;
	GetEngine()->GetThreadPool()->AddWork(submitWork);
}

void MVulkanDevice::Update()
{
	MIDevice::Update();

	CheckFrameFinish();

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

	const VkFormatProperties formatProperties = m_pPhysicalDevice->GetFormatProperties(pBuffer->m_VkTextureFormat);

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
	TransitionLayoutBarrier(imageMemoryBarrier, image, oldLayout, newLayout, subresourceRange);

	const VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	const VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

	vkCmdPipelineBarrier(commandBuffer, srcStageFlags, destStageFlags, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &imageMemoryBarrier);
}

void MVulkanDevice::TransitionLayoutBarrier(VkImageMemoryBarrier& imageMemoryBarrier, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange) const
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
	imageMemoryBarrier.srcAccessMask = GetAspectFlags(oldLayout);
	imageMemoryBarrier.dstAccessMask = GetAspectFlags(newLayout);
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
		return VK_NULL_HANDLE;
	}

	return imageView;
}

void MVulkanDevice::CreateImage(uint32_t nWidth, uint32_t nHeight, uint32_t nDepth, const uint32_t& unMipmap, const uint32_t& unLayerCount, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout defaultLayout, VkImage& image, VkDeviceMemory& imageMemory, VkImageCreateFlags createFlag, VkImageType imageType)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = imageType;
	imageInfo.flags = createFlag;
	imageInfo.extent.width = VALUE_MAX(nWidth, 1);
	imageInfo.extent.height = VALUE_MAX(nHeight, 1);
	imageInfo.extent.depth = VALUE_MAX(nDepth, 1);
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
	allocInfo.memoryTypeIndex = m_pPhysicalDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);

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

VkFragmentShadingRateCombinerOpKHR MVulkanDevice::GetShadingRateCombinerOp(MEShadingRateCombinerOp op) const
{
	MORTY_ASSERT(static_cast<VkFragmentShadingRateCombinerOpKHR>(MEShadingRateCombinerOp::Keep) == VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR);
	MORTY_ASSERT(static_cast<VkFragmentShadingRateCombinerOpKHR>(MEShadingRateCombinerOp::Replace) == VK_FRAGMENT_SHADING_RATE_COMBINER_OP_REPLACE_KHR);
	MORTY_ASSERT(static_cast<VkFragmentShadingRateCombinerOpKHR>(MEShadingRateCombinerOp::Min) == VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MIN_KHR);
	MORTY_ASSERT(static_cast<VkFragmentShadingRateCombinerOpKHR>(MEShadingRateCombinerOp::Max) == VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MAX_KHR);
	MORTY_ASSERT(static_cast<VkFragmentShadingRateCombinerOpKHR>(MEShadingRateCombinerOp::Mul) == VK_FRAGMENT_SHADING_RATE_COMBINER_OP_MUL_KHR);

	return static_cast<VkFragmentShadingRateCombinerOpKHR>(op);
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
	allocInfo.memoryTypeIndex = m_pPhysicalDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);

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

bool MVulkanDevice::GetDeviceFeatureSupport(MEDeviceFeature feature) const
{
	return m_pPhysicalDevice->GetDeviceFeatureSupport(feature);
}

bool MVulkanDevice::MultiDrawIndirectSupport() const
{
	return m_pPhysicalDevice->MultiDrawIndirectSupport();
}

VkInstance MVulkanDevice::GetVkInstance() const
{
	return m_pPhysicalDevice->m_VkInstance;
}

const MVulkanPhysicalDevice* MVulkanDevice::GetPhysicalDevice() const
{
	return m_pPhysicalDevice.get();
}

MVulkanSecondaryRenderCommand* MVulkanDevice::CreateChildCommand(MVulkanPrimaryRenderCommand* pParentCommand)
{
	MORTY_UNUSED(pParentCommand);

	MVulkanSecondaryRenderCommand* pSecondaryCommand = new MVulkanSecondaryRenderCommand();
	pSecondaryCommand->m_pDevice = this;

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandPool = m_VkPresetCommandPool;
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
			const bool bCommandFinished = pCommand->IsFinished();
			if (bCommandFinished)
			{
				pCommand->OnCommandFinished();
			}

			bFinished &= bCommandFinished;
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
				{
					m_pRecycleBin = nullptr;
				}

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
	allocInfo.commandPool = m_VkTemporaryCommandPool;
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

	vkQueueSubmit(m_VkTemporaryQueue, 1, &submitInfo, VK_NULL_HANDLE);

	const VkResult result = vkQueueWaitIdle(m_VkTemporaryQueue);
	MORTY_ASSERT(result == VkResult::VK_SUCCESS);

	vkFreeCommandBuffers(m_VkDevice, m_VkTemporaryCommandPool, 1, &commandBuffer);
}

bool MVulkanDevice::InitLogicalDevice()
{
	m_VkDevice = m_pPhysicalDevice->CreateLogicalDevice();
	if (!m_VkDevice)
	{
		return false;
	}

	vkGetDeviceQueue(m_VkDevice, m_pPhysicalDevice->m_nGraphicsFamilyIndex, 0, &m_VkPresetQueue);
	vkGetDeviceQueue(m_VkDevice, m_pPhysicalDevice->m_nGraphicsFamilyIndex, 1, &m_VkTemporaryQueue);

	return true;
}

bool MVulkanDevice::InitCommandPool()
{

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = m_pPhysicalDevice->m_nGraphicsFamilyIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

	MORTY_ASSERT(vkCreateCommandPool(m_VkDevice, &poolInfo, nullptr, &m_VkPresetCommandPool) == VK_SUCCESS);
		
	MORTY_ASSERT(vkCreateCommandPool(m_VkDevice, &poolInfo, nullptr, &m_VkTemporaryCommandPool) == VK_SUCCESS);

	return true;
}

int MVulkanDevice::FindQueuePresentFamilies(VkSurfaceKHR surface) const
{
	return m_pPhysicalDevice->FindQueuePresentFamilies(surface);
}

VkPhysicalDeviceProperties MVulkanDevice::GetPhysicalDeviceProperties() const
{
	return m_pPhysicalDevice->m_VkPhysicalDeviceProperties;
}

Vector2i MVulkanDevice::GetShadingRateTextureTexelSize() const
{
	const auto vkSize = GetPhysicalDevice()->m_VkFragmentShadingRateProperties.maxFragmentShadingRateAttachmentTexelSize;
	return Vector2i(vkSize.width, vkSize.height);
}

#endif
