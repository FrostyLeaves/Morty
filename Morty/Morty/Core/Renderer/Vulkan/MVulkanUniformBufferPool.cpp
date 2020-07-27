#include "MVulkanUniformBufferPool.h"

#include "MFunction.h"
#include "MVulkanDevice.h"
#include "Shader/MShaderParam.h"


MVulkanUniformBufferPool::MVulkanUniformBufferPool(MVulkanDevice* pDevice)
	: m_pDevice(pDevice)
	, m_unMinUboAlignment(m_pDevice->m_VkPhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment)
	, m_unBufferMemorySize(0)
	, m_VkBuffer(VK_NULL_HANDLE)
	, m_VkDeviceMemory(VK_NULL_HANDLE)
	, m_pMemoryMapping(nullptr)
{

}

MVulkanUniformBufferPool::~MVulkanUniformBufferPool()
{

}

bool MVulkanUniformBufferPool::Initialize()
{
	uint32_t unAllowMemorySize = 32 * 1024 * 1024;

	m_pDevice->m_BufferManager.GenerateBuffer(unAllowMemorySize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_VkBuffer, m_VkDeviceMemory);

	void* pData = nullptr;
	vkMapMemory(m_pDevice->m_VkDevice, m_VkDeviceMemory, 0, unAllowMemorySize, 0, &pData);
	m_pMemoryMapping = (MByte*)pData;

	return true;
}

void MVulkanUniformBufferPool::Release()
{
	vkUnmapMemory(m_pDevice->m_VkDevice, m_VkDeviceMemory);

	m_pDevice->m_BufferManager.DestroyBuffer(m_VkBuffer, m_VkDeviceMemory);
}

bool MVulkanUniformBufferPool::AllowBufferMemory(MShaderConstantParam* pParam)
{
	if (!pParam)
		return false;

	if (pParam->m_VkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		return AllowUniformBufferMemory(pParam);

	if (pParam->m_VkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		return AllowDynamicUniformBufferMemory(pParam);

	return false;
}

bool MVulkanUniformBufferPool::AllowUniformBufferMemory(MShaderConstantParam* pParam)
{
	uint32_t unSize = pParam->var.GetSize();

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{

		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;

		if (!m_pDevice->m_BufferManager.GenerateBuffer(unSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffer, memory))
		{
			for (uint32_t n = 0; n < i; ++n)
				FreeUniformBufferMemory(pParam);
			
			return false;
		}

		pParam->m_VkBuffer[i] = buffer;
		pParam->m_VkBufferMemory[i] = memory;
		pParam->m_unMemoryOffset[i] = 0;
		pParam->m_unVkMemorySize = unSize;
	
		void* pData = nullptr;
		vkMapMemory(m_pDevice->m_VkDevice, memory, 0, unSize, 0, &pData);
		pParam->m_pMemoryMapping[i] = (MByte*)pData;
	}

	return true;
}

bool MVulkanUniformBufferPool::AllowDynamicUniformBufferMemory(MShaderConstantParam* pParam)
{
	if (pParam->m_VkBuffer != VK_NULL_HANDLE)
		return false;

	uint32_t unVariantSize = pParam->var.GetSize();

	if (unVariantSize > 0) {
		unVariantSize = (unVariantSize + m_unMinUboAlignment - 1) & ~(m_unMinUboAlignment - 1);
	}

	MemoryInfo allowInfo[M_BUFFER_NUM];

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		if (!AllowMemory(unVariantSize, allowInfo[i]))
		{
			for (uint32_t n = 0; n < i; ++n)
				FreeMemory(allowInfo[n]);
			
			return false;
		}
	}

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		m_tUsingMemory[i][pParam] = allowInfo[i];

		pParam->m_VkBuffer[i] = m_VkBuffer;
		pParam->m_VkBufferMemory[i] = m_VkDeviceMemory;
		pParam->m_unMemoryOffset[i] = allowInfo[i].begin + unVariantSize * i;
		pParam->m_pMemoryMapping[i] = m_pMemoryMapping;
		pParam->m_unVkMemorySize = unVariantSize;
	}

	return true;
}

void MVulkanUniformBufferPool::FreeBufferMemory(MShaderConstantParam* pParam)
{
	if (!pParam)
		return ;

	if (VK_NULL_HANDLE == pParam->m_VkBuffer[0])
		return;

	if (pParam->m_VkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		return FreeUniformBufferMemory(pParam);

	if (pParam->m_VkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		return FreeDynamicUniformBufferMemory(pParam);
}

void MVulkanUniformBufferPool::FreeUniformBufferMemory(MShaderConstantParam* pParam)
{
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		vkUnmapMemory(m_pDevice->m_VkDevice, pParam->m_VkBufferMemory[i]);
		pParam->m_pMemoryMapping[i] = nullptr;

		m_pDevice->m_BufferManager.DestroyBufferLater(i, pParam->m_VkBuffer[i]);
		m_pDevice->m_BufferManager.DestroyDeviceMemoryLater(i, pParam->m_VkBufferMemory[i]);

		pParam->m_VkBuffer[i] = VK_NULL_HANDLE;
		pParam->m_VkBufferMemory[i] = VK_NULL_HANDLE;
		pParam->m_unMemoryOffset[i] = 0;
	}
}

void MVulkanUniformBufferPool::FreeDynamicUniformBufferMemory(MShaderConstantParam* pParam)
{
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		MemoryInfo info = m_tUsingMemory[i][pParam];
		m_tUsingMemory[i].erase(pParam);

		FreeMemory(info);
	}
}

bool MVulkanUniformBufferPool::AllowMemory(const uint32_t& unVariantSize, MemoryInfo& info)
{
	std::vector<MemoryInfo>::iterator biggestIter = m_vFreeMemory.begin();
	std::vector<MemoryInfo>::iterator bestIter = m_vFreeMemory.end();
	for (auto iter = m_vFreeMemory.begin(); iter != m_vFreeMemory.end(); ++iter)
	{
		if (iter->size == unVariantSize)
		{
			bestIter = iter;
			break;
		}

		if (iter->size > biggestIter->size)
			biggestIter = iter;
	}

	if (bestIter == m_vFreeMemory.end() && biggestIter->size >= unVariantSize)
		bestIter = biggestIter;

	if (bestIter == m_vFreeMemory.end())
	{

		return false;
	}

	if (bestIter->size == unVariantSize)
	{
		info = *bestIter;
		m_vFreeMemory.erase(bestIter);

	}
	else if (bestIter->size > unVariantSize)
	{
		info = *bestIter;

		bestIter->begin += unVariantSize;
		bestIter->size -= unVariantSize;
	}
	else
	{
		return false;
	}

	return true;
}

void MVulkanUniformBufferPool::FreeMemory(MemoryInfo& info)
{
	std::vector<MemoryInfo>::iterator iter = std::lower_bound(m_vFreeMemory.begin(), m_vFreeMemory.end(), info);

	if (iter == m_vFreeMemory.end())
	{
		MemoryInfo& back = m_vFreeMemory.back();
		if (back.begin + back.size == info.begin)
			back.size += info.size;
		else
			m_vFreeMemory.push_back(info);
	}
	else
	{
		MemoryInfo& prev = *(iter - 1);
		MemoryInfo& next = *(iter + 1);
		if (prev.begin + prev.size == info.begin)
		{
			prev.size += info.size;
		}
		else if (info.begin + info.size == next.begin)
		{
			next.begin -= info.size;
			next.size += info.size;
		}
		else
			m_vFreeMemory.insert(iter, info);
	}
}
