#include "MVulkanDevice.h"
#include "MMesh.h"
#include "MEngine.h"
#include "MTexture.h"
#include "MResource.h"
#include "MFileHelper.h"
#include "MShaderParam.h"
#include "MVertexBuffer.h"
#include "MVulkanRenderCommand.h"

#ifdef max
#undef max
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
};


MVulkanDevice::MVulkanDevice()
	: MIDevice()
	, m_VkInstance(VK_NULL_HANDLE)
	, m_VkPhysicalDevice(VK_NULL_HANDLE)
	, m_VkPhysicalDeviceProperties({})
	, m_VkDevice(VK_NULL_HANDLE)
	, m_VkGraphicsQueue(VK_NULL_HANDLE)
	, m_VkCommandPool(VK_NULL_HANDLE)
	, m_pRecycleBin(nullptr)
	, m_PipelineManager(this)
	, m_ShaderCompiler(this)
	, m_BufferPool(this)
	, m_VkDepthTextureFormat(VK_FORMAT_D32_SFLOAT_S8_UINT)
	, m_VkDefaultSampler(VK_NULL_HANDLE)
	, m_VkLessEqualSampler(VK_NULL_HANDLE)
	, m_VkDescriptorPool(VK_NULL_HANDLE)
{

}

MVulkanDevice::~MVulkanDevice()
{

}

bool MVulkanDevice::Initialize()
{
#ifdef MORTY_ANDROID
	if (!InitVulkan())
		return false;
#endif

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

	m_ShaderCompiler.Initialize();

	return true;
}

void MVulkanDevice::Release()
{
	m_PipelineManager.Release();
	m_BufferPool.Release();
	m_ShaderDefaultTexture.DestroyBuffer(this);

	for (auto pr : m_tRecycleBin)
	{
		pr.second->Release();
		delete pr.second;
		pr.second = nullptr;
	}
	m_tRecycleBin.clear();
	m_pRecycleBin = nullptr;

	m_pFastRecycleBin->Release();
	delete m_pFastRecycleBin;
	m_pFastRecycleBin = nullptr;

	vkDestroySampler(m_VkDevice, m_VkDefaultSampler, nullptr);
	vkDestroySampler(m_VkDevice, m_VkLessEqualSampler, nullptr);
	vkDestroyDescriptorPool(m_VkDevice, m_VkDescriptorPool, nullptr);
	vkDestroyCommandPool(m_VkDevice, m_VkCommandPool, nullptr);
	vkDestroyDevice(m_VkDevice, nullptr);
	vkDestroyInstance(m_VkInstance, NULL);
}

int MVulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_VkPhysicalDevice, &memProperties);

	for (int i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	return MGlobal::M_INVALID_INDEX;
}

int MVulkanDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (int i = 0; i < candidates.size(); ++i)
	{
		const VkFormat& format = candidates[i];

		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(m_VkPhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return i;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return i;
		}
	}

	return MGlobal::M_INVALID_INDEX;
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
	case METextureLayout::ER32:
		return VK_FORMAT_R32_SFLOAT;
		break;
	case METextureLayout::EDepth:
		return m_VkDepthTextureFormat;
		break;

	case METextureLayout::ERGBA8:
		return VK_FORMAT_R8G8B8A8_SRGB;
		break;

	case METextureLayout::ERGBA16:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
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
		return VK_IMAGE_ASPECT_DEPTH_BIT;

	return VK_IMAGE_ASPECT_COLOR_BIT;
}

VkImageLayout MVulkanDevice::GetImageLayout(MTexture* pTexture)
{
	if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderPresent)
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	return VK_IMAGE_LAYOUT_UNDEFINED;
}

int MVulkanDevice::GetPimpapSize(MTexture* pTexture)
{
	uint32_t unMipmap = 1;

	if (pTexture->GetMipmapsEnable() && pTexture->GetRenderUsage() == METextureRenderUsage::EUnknow)
	{
		unMipmap = static_cast<uint32_t>(std::floor(std::log2(std::max(pTexture->GetSize().x, pTexture->GetSize().y)))) + 1;
	}

	return unMipmap;
}

MVulkanObjectRecycleBin* MVulkanDevice::GetRecycleBin()
{
	return m_pRecycleBin ? m_pRecycleBin : m_pFastRecycleBin;
}

bool MVulkanDevice::InitDepthFormat()
{
	std::vector<VkFormat> formats = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

	int index = FindSupportedFormat(formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	if (index == MGlobal::M_INVALID_INDEX)
		return false;

	m_VkDepthTextureFormat = formats[index];
	return true;
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

	if (vkCreateSampler(m_VkDevice, &samplerInfo, nullptr, &m_VkDefaultSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	samplerInfo.compareEnable = VK_TRUE;
	samplerInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	if (vkCreateSampler(m_VkDevice, &samplerInfo, nullptr, &m_VkLessEqualSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	return true;
}

bool MVulkanDevice::InitializeRecycleBin()
{
	m_pFastRecycleBin = new MVulkanObjectRecycleBin(this);
	m_pFastRecycleBin->Initialize();
	return true;
}

bool MVulkanDevice::InitDescriptorPool()
{
	uint32_t unSwapChainNum = 500;

	std::vector<VkDescriptorPoolSize> vPoolSize(5);
	vPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	vPoolSize[0].descriptorCount = unSwapChainNum;

	vPoolSize[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	vPoolSize[1].descriptorCount = unSwapChainNum;

	vPoolSize[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	vPoolSize[2].descriptorCount = unSwapChainNum;

	vPoolSize[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	vPoolSize[3].descriptorCount = unSwapChainNum;

	vPoolSize[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	vPoolSize[4].descriptorCount = unSwapChainNum;

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

void MVulkanDevice::GenerateVertex(MVertexBuffer* ppVertexBuffer, MIMesh* pMesh, const bool& bModifiable /*= false*/)
{
	void* data = nullptr;

	VkDeviceSize bufferSize = static_cast<uint64_t>(pMesh->GetVerticesLength()) * static_cast<uint64_t>(pMesh->GetVertexStructSize());
	VkDeviceSize indexBufferSize = sizeof(uint32_t) * pMesh->GetIndicesLength();

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	if (bModifiable)
	{
		GenerateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);
		vkMapMemory(m_VkDevice, vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, pMesh->GetVertices(), (size_t)bufferSize);
		vkUnmapMemory(m_VkDevice, vertexBufferMemory);

		GenerateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT , indexBuffer, indexBufferMemory);
		vkMapMemory(m_VkDevice, indexBufferMemory, 0, indexBufferSize, 0, &data);
		memcpy(data, pMesh->GetIndices(), (size_t)indexBufferSize);
		vkUnmapMemory(m_VkDevice, indexBufferMemory);
	}
	else
	{
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		GenerateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		vkMapMemory(m_VkDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, pMesh->GetVertices(), (size_t)bufferSize);
		vkUnmapMemory(m_VkDevice, stagingBufferMemory);

		
		GenerateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		DestroyBuffer(stagingBuffer, stagingBufferMemory);


		VkBuffer stagingIdxBuffer;
		VkDeviceMemory stagingIdxBufferMemory;
		GenerateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingIdxBuffer, stagingIdxBufferMemory);

		vkMapMemory(m_VkDevice, stagingIdxBufferMemory, 0, indexBufferSize, 0, &data);
		memcpy(data, pMesh->GetIndices(), (size_t)indexBufferSize);
		vkUnmapMemory(m_VkDevice, stagingIdxBufferMemory);

		GenerateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		CopyBuffer(stagingIdxBuffer, indexBuffer, indexBufferSize);

		DestroyBuffer(stagingIdxBuffer, stagingIdxBufferMemory);
	}

	ppVertexBuffer->m_VkVertexBuffer = vertexBuffer;
	ppVertexBuffer->m_VkVertexBufferMemory = vertexBufferMemory;
	ppVertexBuffer->m_VkIndexBuffer = indexBuffer;
	ppVertexBuffer->m_VkIndexBufferMemory = indexBufferMemory;
}

void MVulkanDevice::DestroyVertex(MVertexBuffer* ppVertexBuffer)
{
	if (!ppVertexBuffer)
		return;

	GetRecycleBin()->DestroyBufferLater(ppVertexBuffer->m_VkVertexBuffer);
	GetRecycleBin()->DestroyDeviceMemoryLater(ppVertexBuffer->m_VkVertexBufferMemory);
	GetRecycleBin()->DestroyBufferLater(ppVertexBuffer->m_VkIndexBuffer);
	GetRecycleBin()->DestroyDeviceMemoryLater(ppVertexBuffer->m_VkIndexBufferMemory);	
}

void MVulkanDevice::UploadVertex(MVertexBuffer* ppVertexBuffer, MIMesh* pMesh)
{
	{
		void* data = nullptr;
		uint32_t unSize = pMesh->GetVerticesLength() * pMesh->GetVertexStructSize();
		vkMapMemory(m_VkDevice, ppVertexBuffer->m_VkVertexBufferMemory, 0, unSize, 0, &data);
		memcpy(data, pMesh->GetVertices(), unSize);
		vkUnmapMemory(m_VkDevice, ppVertexBuffer->m_VkVertexBufferMemory);
	}
	{
		void* data = nullptr;
		uint32_t unSize = pMesh->GetIndicesLength() * sizeof(uint32_t);
		vkMapMemory(m_VkDevice, ppVertexBuffer->m_VkIndexBufferMemory, 0, unSize, 0, &data);
		memcpy(data, pMesh->GetIndices(), unSize);
		vkUnmapMemory(m_VkDevice, ppVertexBuffer->m_VkIndexBufferMemory);
	}
}

void MVulkanDevice::GenerateTexture(MTexture* pTexture, MByte* pData)
{
	uint64_t width = pTexture->GetSize().x;
	uint64_t height = pTexture->GetSize().y;
	VkFormat format = GetFormat(pTexture->GetTextureLayout());

	VkImageUsageFlags usageFlags = GetUsageFlags(pTexture);
	VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkImageAspectFlags aspectFlgas = GetAspectFlags(pTexture);
	VkImageLayout defaultLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkDeviceSize imageSize = static_cast<uint64_t>(MTexture::GetImageMemorySize(pTexture->GetTextureLayout())) * width * height;


	VkImage textureImage = VK_NULL_HANDLE;
	VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;

	int unMipmap = GetPimpapSize(pTexture);

	if (pTexture->GetRenderUsage() == METextureRenderUsage::ERenderPresent)
	{
		VkImageSubresourceRange vkSubresourceRange = {};
		vkSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vkSubresourceRange.baseMipLevel = 0;
		vkSubresourceRange.levelCount = unMipmap;
		vkSubresourceRange.layerCount = 1;
		TransitionImageLayout(pTexture->m_VkTextureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vkSubresourceRange);
	}
	else
	{

		if (pTexture->m_VkTextureImage)
		{
			assert(false);
		}

		CreateImage(width, height, unMipmap, format, VK_IMAGE_TILING_OPTIMAL, usageFlags, memoryFlags, defaultLayout, textureImage, textureImageMemory);

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
			vkSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			vkSubresourceRange.baseMipLevel = 0;
			vkSubresourceRange.levelCount = unMipmap;
			vkSubresourceRange.layerCount = 1;

			TransitionImageLayout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkSubresourceRange);
			CopyImageBuffer(stagingBuffer, textureImage, width, height, 1);
			TransitionImageLayout(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vkSubresourceRange);

			DestroyBuffer(stagingBuffer, stagingBufferMemory);
		}

		pTexture->m_VkTextureImage = textureImage;
		pTexture->m_unMipmapLevel = unMipmap;
		pTexture->m_VkTextureImageMemory = textureImageMemory;
		pTexture->m_VkTextureFormat = format;
		pTexture->m_VkImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}

	pTexture->m_VkImageView = CreateImageView(pTexture->m_VkTextureImage, pTexture->m_VkTextureFormat, aspectFlgas, unMipmap);
	

	if (unMipmap > 1)
	{
		GenerateMipmaps(pTexture, unMipmap);
	}

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
	shaderStageInfo.stage = pShader->GetType() == MEShaderType::EVertex ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfo.module = shaderModule;
	shaderStageInfo.pName = pShader->GetType() == MEShaderType::EVertex ? "VS" : "PS";


	shaderStageInfo.pSpecializationInfo = nullptr;

	MShaderBuffer* pShaderBuffer = nullptr;
	if (MEShaderType::EVertex == pShader->GetType())
	{
		MVertexShaderBuffer* pBuffer = new MVertexShaderBuffer();
		m_ShaderCompiler.GetVertexInputState(compiler, pBuffer);
		pShaderBuffer = pBuffer;
	}
	else if (MEShaderType::EPixel == pShader->GetType())
	{
		MPixelShaderBuffer* pBuffer = new MPixelShaderBuffer();
		pShaderBuffer = pBuffer;
	}


	pShaderBuffer->m_VkShaderModule = shaderModule;
	pShaderBuffer->m_VkShaderStageInfo = shaderStageInfo;
	m_ShaderCompiler.GetShaderParam(compiler, pShaderBuffer);

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

bool MVulkanDevice::GenerateShaderParamSet(MShaderParamSet* pParamSet)
{
	if (!pParamSet)
		return false;

	m_PipelineManager.GenerateShaderParamSet(pParamSet);

	return true;
}

void MVulkanDevice::DestroyShaderParamSet(MShaderParamSet* pParamSet)
{
	if (pParamSet)
	{
		m_PipelineManager.DestroyShaderParamSet(pParamSet);
	}
}

bool MVulkanDevice::GenerateShaderParamBuffer(MShaderConstantParam* pParam)
{
	if (!pParam)
		return false;

	if (VK_NULL_HANDLE != pParam->m_VkBuffer)
		DestroyShaderParamBuffer(pParam);


	return m_BufferPool.AllowBufferMemory(pParam);
}

void MVulkanDevice::DestroyShaderParamBuffer(MShaderConstantParam* pParam)
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

	uint32_t unBackNum = pRenderPass->m_vBackDesc.size();

	std::vector<VkAttachmentDescription> vAttachmentDesc;

	for (uint32_t i = 0; i < unBackNum; ++i)
	{
		MTexture* pBackTexture = pRenderPass->m_vBackTextures[i];
		if (!pBackTexture)
		{
			GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateRenderPass error: bt == nullptr");
			return false;
		}

		if (pBackTexture->m_VkTextureImage == VK_NULL_HANDLE)
		{
			pBackTexture->GenerateBuffer(this);
		}

		vAttachmentDesc.push_back({});
		VkAttachmentDescription& colorAttachment = vAttachmentDesc.back();


		if (pRenderPass->m_vBackDesc[i].bClearWhenRender)
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		else
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

		colorAttachment.format = pBackTexture->m_VkTextureFormat;

		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		colorAttachment.initialLayout = pBackTexture->m_VkImageLayout;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (pBackTexture->GetRenderUsage() == METextureRenderUsage::ERenderPresent)
		{
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}
	}

	MTexture* pDepthTexture = pRenderPass->m_pDepthTexture;
	if (pDepthTexture)
	{
		if (pDepthTexture->m_VkTextureImage == VK_NULL_HANDLE)
		{
			pDepthTexture->GenerateBuffer(this);
		}

		vAttachmentDesc.push_back({});
		VkAttachmentDescription& colorAttachment = vAttachmentDesc.back();

		if (pRenderPass->m_DepthDesc.bClearWhenRender)
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		else
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;

		colorAttachment.format = m_VkDepthTextureFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;			//for Shader
	}


	std::vector<VkSubpassDescription> vSubpass;

	std::vector<std::vector<VkAttachmentReference>> vOutAttachmentRef;
	std::vector<std::vector<VkAttachmentReference>> vOutDepthAttachmentRef;
	std::vector<std::vector<VkAttachmentReference>> vInAttachmentRef;
	std::vector<std::vector<uint32_t>> vUnusedAttachmentRef;

	// default subpass
	if (pRenderPass->m_vSubpass.empty())
	{
		vOutAttachmentRef.resize(1);
		vOutDepthAttachmentRef.resize(1);

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

		for (uint32_t nSubpassIdx = 0; nSubpassIdx < unSubpassNum; ++nSubpassIdx)
		{
			MSubpass& subpass = pRenderPass->m_vSubpass[nSubpassIdx];

			vSubpass.push_back(VkSubpassDescription());
			VkSubpassDescription& vkSubpass = vSubpass.back();
			vkSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

			std::set<uint32_t> vUsedAttachIndex;

			for (uint32_t i = 0; i < subpass.m_vOutputIndex.size(); ++i)
			{
				uint32_t nBackIdx = subpass.m_vOutputIndex[i];

				vOutAttachmentRef[nSubpassIdx].push_back({});
				VkAttachmentReference& vkAttachRef = vOutAttachmentRef[nSubpassIdx].back();

				vkAttachRef.attachment = nBackIdx;
				vkAttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
	for (uint32_t nSubpassIdx = 1; nSubpassIdx < vSubpass.size(); ++nSubpassIdx)
	{
		for (int nDependantIdx = 0; nDependantIdx < nSubpassIdx; ++nDependantIdx)
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


	if (vkCreateRenderPass(m_VkDevice, &renderPassInfo, nullptr, &pRenderPass->m_VkRenderPass) != VK_SUCCESS)
	{
		return false;
	}

	m_PipelineManager.RegisterRenderPass(pRenderPass);

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
	Vector2 v2FrameBufferSize = Vector2(0.0, 0.0);

	pRenderPass->m_VkFrameBuffer;
	std::vector<VkImageView> vAttachmentViews;

	uint32_t unBackNum = pRenderPass->m_vBackDesc.size();
	for (uint32_t backIdx = 0; backIdx < unBackNum; ++backIdx)
	{
		MTexture* pBackTexture = pRenderPass->m_vBackTextures[backIdx];
		if (!pBackTexture)
		{
			GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateRenderPass error: bt == nullptr");
			return false;
		}

		if (v2FrameBufferSize != pBackTexture->GetSize())
		{
			if (v2FrameBufferSize.Length() < 1e-6)
				v2FrameBufferSize = pBackTexture->GetSize();
			else
			{
				GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateRenderPass error: different size");
				return false;
			}
		}

		v2FrameBufferSize = pBackTexture->GetSize();

		if (pBackTexture->m_VkImageView == VK_NULL_HANDLE)
		{
			pBackTexture->GenerateBuffer(this);
		}

		vAttachmentViews.push_back(pBackTexture->m_VkImageView);
	}

	MTexture* pDepthTexture = pRenderPass->m_pDepthTexture;

	if (pDepthTexture)
	{
		if (v2FrameBufferSize != pDepthTexture->GetSize())
		{
			if (v2FrameBufferSize.Length() < 1e-6)
				v2FrameBufferSize = pDepthTexture->GetSize();
			else
			{
				GetEngine()->GetLogger()->Error("MVulkanDevice::GenerateRenderPass error: different size");
				return false;
			}
		}

		if (!pDepthTexture->m_VkImageView)
		{
			pDepthTexture->GenerateBuffer(this);
		}

		vAttachmentViews.push_back(pDepthTexture->m_VkImageView);
	}


	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	//This renderpass only used to match format.
	framebufferInfo.renderPass = pRenderPass->m_VkRenderPass;

	framebufferInfo.attachmentCount = vAttachmentViews.size();
	framebufferInfo.pAttachments = vAttachmentViews.data();
	framebufferInfo.width = v2FrameBufferSize.x;
	framebufferInfo.height = v2FrameBufferSize.y;
	framebufferInfo.layers = 1;


	pRenderPass->m_vkExtent2D.width = v2FrameBufferSize.x;
	pRenderPass->m_vkExtent2D.height = v2FrameBufferSize.y;
	vkCreateFramebuffer(m_VkDevice, &framebufferInfo, nullptr, &pRenderPass->m_VkFrameBuffer);


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
}

bool MVulkanDevice::RegisterMaterial(MMaterial* pMaterial)
{
	return m_PipelineManager.RegisterMaterial(pMaterial);
}

bool MVulkanDevice::UnRegisterMaterial(MMaterial* pMaterial)
{
	return m_PipelineManager.UnRegisterMaterial(pMaterial);
}

MIRenderCommand* MVulkanDevice::CreateRenderCommand()
{
	MVulkanRenderCommand* pCommand = new MVulkanRenderCommand();
	pCommand->m_pDevice = this;

	//New CommandBuffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_VkCommandPool;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(m_VkDevice, &allocInfo, &pCommand->m_VkCommandBuffer);


	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	vkCreateFence(m_VkDevice, &fenceInfo, nullptr, &pCommand->m_VkRenderFinishedFence);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCreateSemaphore(m_VkDevice, &semaphoreInfo, nullptr, &pCommand->m_VkRenderFinishedSemaphore);


	return pCommand;
}

void MVulkanDevice::RecoveryRenderCommand(MIRenderCommand* pRenderCommand)
{
	MVulkanRenderCommand* pCommand = static_cast<MVulkanRenderCommand*>(pRenderCommand);
	
	if (!pCommand)
		return;

	//TODO cancel wait.
	while (vkGetFenceStatus(m_VkDevice, pCommand->m_VkRenderFinishedFence) != VK_SUCCESS);

	if (pCommand->m_VkCommandBuffer)
	{
		GetRecycleBin()->DestroyCommandBufferLater(pCommand->m_VkCommandBuffer);
		pCommand->m_VkCommandBuffer = VK_NULL_HANDLE;
	}

	if (pCommand->m_VkRenderFinishedFence)
	{
		GetRecycleBin()->DestroyFenceLater(pCommand->m_VkRenderFinishedFence);
		pCommand->m_VkRenderFinishedFence = VK_NULL_HANDLE;
	}

	if (pCommand->m_VkRenderFinishedSemaphore)
	{
		GetRecycleBin()->DestroySemaphoreLater(pCommand->m_VkRenderFinishedSemaphore);
		pCommand->m_VkRenderFinishedSemaphore = VK_NULL_HANDLE;
	}

	//TODO Recovery
	delete pCommand;
}

bool MVulkanDevice::IsFinishedCommand(MIRenderCommand* pCommand)
{
	if (MVulkanRenderCommand* pVulkanCommand = static_cast<MVulkanRenderCommand*>(pCommand))
	{
		return VK_SUCCESS == vkGetFenceStatus(m_VkDevice, pVulkanCommand->m_VkRenderFinishedFence);
	}

	return false;
}

void MVulkanDevice::NewFrame(const uint32_t& nIdx)
{
	m_tRecycleBin[nIdx] = m_pRecycleBin = new MVulkanObjectRecycleBin(this);
	m_pRecycleBin->Initialize();
}

void MVulkanDevice::FrameFinish(const uint32_t& nIdx)
{
	if (MVulkanObjectRecycleBin* pRecycleBin = m_tRecycleBin[nIdx])
	{
		if (m_pRecycleBin == pRecycleBin)
			m_pRecycleBin = nullptr;

		m_tRecycleBin.erase(nIdx);

		pRecycleBin->Release();
		delete pRecycleBin;
		pRecycleBin = nullptr;
	}

	if (m_pFastRecycleBin)
	{
		m_pFastRecycleBin->EmptyTrash();
	}
}

void MVulkanDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = BeginCommands();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

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

	TransitionImageLayout(pSource->m_VkTextureImage, pSource->m_VkImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, range, commandBuffer);
	TransitionImageLayout(pDestination->m_VkTextureImage, pDestination->m_VkImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, range, commandBuffer);

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


	TransitionImageLayout(pSource->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, pSource->m_VkImageLayout, range, commandBuffer);
	TransitionImageLayout(pDestination->m_VkTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, pDestination->m_VkImageLayout, range, commandBuffer);


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

	VkImageSubresourceRange vkSubresourceRange = {};
	vkSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkSubresourceRange.baseMipLevel = 0;
	vkSubresourceRange.levelCount = unMipLevels;
	vkSubresourceRange.layerCount = 1;
	TransitionImageLayout(pBuffer->m_VkTextureImage, pBuffer->m_VkImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkSubresourceRange, commandBuffer);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = pBuffer->m_VkTextureImage;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
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
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

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

void MVulkanDevice::TransitionImageLayout(VkImage image,VkImageLayout oldLayout,VkImageLayout newLayout,VkImageSubresourceRange subresourceRange, VkCommandBuffer buffer/* = VK_NULL_HANDLE*/)
{
	VkCommandBuffer commandBuffer = buffer ? buffer : BeginCommands();

	// Create an image barrier object
	VkImageMemoryBarrier imageMemoryBarrier = {};
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
		imageMemoryBarrier.srcAccessMask
			= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
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
		assert(false);
	}

	switch (newLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.srcAccessMask |= VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.dstAccessMask
			= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (imageMemoryBarrier.srcAccessMask == 0)
		{
			imageMemoryBarrier.srcAccessMask
				= VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	default:
		assert(false);
	}

	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

	vkCmdPipelineBarrier(commandBuffer, srcStageFlags, destStageFlags, 0, 0, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 1, &imageMemoryBarrier);

	if (!buffer)
	{
		EndCommands(commandBuffer);
	}
}

VkImageView MVulkanDevice::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, const uint32_t& unMipmap)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = unMipmap;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(m_VkDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		VK_NULL_HANDLE;
	}

	return imageView;
}

void MVulkanDevice::CreateImage(const uint32_t& unWidth, const uint32_t& unHeight, const uint32_t& unMipmap, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout defaultLayout, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = VALUE_MAX(unWidth, 1);
	imageInfo.extent.height = VALUE_MAX(unHeight, 1);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = unMipmap;
	imageInfo.arrayLayers = 1;
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

	if (MGlobal::M_INVALID_INDEX == allocInfo.memoryTypeIndex)
	{
		assert(false);
	}

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(m_VkDevice, image, imageMemory, 0);
}

void MVulkanDevice::CreateImageCube(const uint32_t& unWidth, const uint32_t& unHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { unWidth, unHeight, 1 };
	imageCreateInfo.usage = usage;
	imageCreateInfo.arrayLayers = 6;
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	vkCreateImage(m_VkDevice, &imageCreateInfo, nullptr, &image);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_VkDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(m_VkDevice, image, imageMemory, 0);
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
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_VkDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (MGlobal::M_INVALID_INDEX == allocInfo.memoryTypeIndex)
	{
		vkDestroyBuffer(m_VkDevice, buffer, nullptr);
		return false;
	}

	if (vkAllocateMemory(m_VkDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
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

VkCommandBuffer MVulkanDevice::BeginCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_VkCommandPool;
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

	vkQueueWaitIdle(m_VkGraphicsQueue);	//��ͣӦ�ó���ֱ������ύ�������е����й���

	vkFreeCommandBuffers(m_VkDevice, m_VkCommandPool, 1, &commandBuffer);
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
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	
#if _DEBUG
	createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
	createInfo.ppEnabledLayerNames = ValidationLayers.data();
#else
	createInfo.enabledLayerCount = 0;
#endif

	std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef MORTY_MACOS
        "VK_EXT_metal_surface",
        "VK_MVK_macos_surface"
#endif
        
#ifdef MORTY_IOS
        "VK_EXT_metal_surface",
        "VK_MVK_ios_surface",
        "VK_MVK_moltenvk"
#endif

#ifdef MORTY_WIN
		"VK_KHR_surface",
		"VK_KHR_win32_surface"
#endif
    };

#if defined(MORTY_WIN)
	enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(MORTY_ANDROID)
	enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(MORTY_IOS)
	//enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

	createInfo.enabledExtensionCount = enabledExtensions.size();
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();

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
			"The call to vkCreateInstance failed. Please make sure "
			"you have a Vulkan installable client driver (ICD) before "
			"continuing.");
		return false;
	}

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
		GetEngine()->GetLogger()->Information("API Version:    %d.%d.%d\n",
			VK_VERSION_MAJOR(m_VkPhysicalDeviceProperties.apiVersion),
			VK_VERSION_MINOR(m_VkPhysicalDeviceProperties.apiVersion),
			VK_VERSION_PATCH(m_VkPhysicalDeviceProperties.apiVersion));

		if (IsDeviceSuitable(vPhysicalDevices[i]))
		{
			m_VkPhysicalDevice = vPhysicalDevices[i];
			break;
		}
	}

	if (m_VkPhysicalDevice == VK_NULL_HANDLE)
	{
		GetEngine()->GetLogger()->Error("Initialize Vulkan Error : m_nPhysicalDeviceIndex == -1");
		return false;
	}


	return true;
}

bool MVulkanDevice::InitLogicalDevice()
{
	int nQueueFamilyIndex = FindQueueGraphicsFamilies(m_VkPhysicalDevice);

	float priorities[] = { 1.0f };	//range 0~1
	VkDeviceQueueCreateInfo queueInfo{};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = NULL;
	queueInfo.flags = 0;
	queueInfo.queueFamilyIndex = nQueueFamilyIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &priorities[0];


	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures(m_VkPhysicalDevice, &deviceFeatures);


	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = NULL;
	deviceInfo.flags = 0;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = DeviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = DeviceExtensions.data();
	deviceInfo.pEnabledFeatures = &deviceFeatures;

	//����Ĭ���˵�ǰ�����壨nQueueFamilyIndex����֧���������͵Ĺ���
	//���ܻ���ڲ�֧������ǰ��Ļ�ϻ��Ƶģ����������Ҫ�ڴ���logicalDevice֮ǰ�������ڵ�surface��Ȼ��check֧��֧�֡�
	//���̫�����ˣ��Ȳ�����
	VkResult result = vkCreateDevice(m_VkPhysicalDevice, &deviceInfo, NULL, &m_VkDevice);
	if (result != VK_SUCCESS)
	{
		GetEngine()->GetLogger()->Error("Initialize Vulkan Error : vkCreateDevice error.");
		return false;
	}

	vkGetDeviceQueue(m_VkDevice, nQueueFamilyIndex, 0, &m_VkGraphicsQueue);

	return true;
}

bool MVulkanDevice::InitCommandPool()
{
	uint32_t unQueueFamilyIndex = FindQueueGraphicsFamilies(m_VkPhysicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = unQueueFamilyIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

	if (vkCreateCommandPool(m_VkDevice, &poolInfo, nullptr, &m_VkCommandPool) != VK_SUCCESS)
		return false;


	return true;
}

bool MVulkanDevice::InitDefaultTexture()
{
	m_ShaderDefaultTexture.SetMipmapsEnable(false);
	m_ShaderDefaultTexture.SetReadable(false);
	m_ShaderDefaultTexture.SetRenderUsage(METextureRenderUsage::EUnknow);
	m_ShaderDefaultTexture.SetShaderUsage(METextureShaderUsage::ESampler);
	m_ShaderDefaultTexture.SetSize(Vector2(4, 4));

	MByte bytes[4 * 4 * 4];
	for (size_t i = 0; i < 4 * 4; i += 4)
	{
		bytes[i + 0] = (uint32_t)(1.0f * 255.0f);
		bytes[i + 1] = (uint32_t)(1.0f * 255.0f);
		bytes[i + 2] = (uint32_t)(1.0f * 255.0f);
		bytes[i + 3] = (uint32_t)(0.0f * 255.0f);
	}
	m_ShaderDefaultTexture.GenerateBuffer(this, bytes);

	return true;
}

bool MVulkanDevice::IsDeviceSuitable(VkPhysicalDevice device)
{
	if (-1 == FindQueueGraphicsFamilies(device))
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

	for (int i = 0; i < unQueueFamilyCount; ++i)
	{
		if (vQueueProperties[i].queueCount > 0 && vQueueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			graphicsFamily = i;

		if (graphicsFamily >= 0)
			break;
	}

	return graphicsFamily;
}

int MVulkanDevice::FindQueuePresentFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
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

bool MVulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

#endif
