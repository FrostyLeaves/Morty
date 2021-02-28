#include "MVulkanBufferPool.h"

#include "MVulkanDevice.h"
#include "Shader/MShaderParam.h"

const uint32_t UinformMemorySize = 32 * 1024 * 1024;
const uint32_t ReadBackMemorySize = 32 * 1024 * 1024;

MVulkanBufferPool::MVulkanBufferPool(MVulkanDevice* pDevice)
	: m_pDevice(pDevice)

	, m_unMinUboAlignment(0)
	, m_unDynamicUniformBufferMemorySize(UinformMemorySize)
	, m_DynamicUniformMemoryPool(UinformMemorySize)
	, m_tDynamicUniformMemory()
	, m_VkDynamicUniformBuffer(VK_NULL_HANDLE)
	, m_VkDynamicUniformMemory(VK_NULL_HANDLE)
	, m_pDynamicUniformMemoryMapping(nullptr)

	, m_unReadBackBufferMemorySize(ReadBackMemorySize)
	, m_ReadBackMemoryPool(ReadBackMemorySize)
	, m_ReadBackIDPool()
	, m_tReadBackMemory()
	, m_VkReadBackBuffer(VK_NULL_HANDLE)
	, m_VkReadBackMemory(VK_NULL_HANDLE)
	, m_pReadBackMemoryMapping(nullptr)
{

}

MVulkanBufferPool::~MVulkanBufferPool()
{

}

bool MVulkanBufferPool::Initialize()
{
	m_unMinUboAlignment = m_pDevice->m_VkPhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment;

	m_pDevice->m_ObjectDestructor.GenerateBuffer(m_unDynamicUniformBufferMemorySize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_VkDynamicUniformBuffer, m_VkDynamicUniformMemory);

	void* pData = nullptr;
	vkMapMemory(m_pDevice->m_VkDevice, m_VkDynamicUniformMemory, 0, m_unDynamicUniformBufferMemorySize, 0, &pData);
	m_pDynamicUniformMemoryMapping = (MByte*)pData;


	m_pDevice->m_ObjectDestructor.GenerateBuffer(m_unReadBackBufferMemorySize, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_VkReadBackBuffer, m_VkReadBackMemory);

	void* pData2 = nullptr;
	vkMapMemory(m_pDevice->m_VkDevice, m_VkReadBackMemory, 0, m_unReadBackBufferMemorySize, 0, &pData2);
	m_pReadBackMemoryMapping = (MByte*)pData2;

	return true;
}

void MVulkanBufferPool::Release()
{
	vkUnmapMemory(m_pDevice->m_VkDevice, m_VkDynamicUniformMemory);
	m_pDevice->m_ObjectDestructor.DestroyBuffer(m_VkDynamicUniformBuffer, m_VkDynamicUniformMemory);

	vkUnmapMemory(m_pDevice->m_VkDevice, m_VkReadBackMemory);
	m_pDevice->m_ObjectDestructor.DestroyBuffer(m_VkReadBackBuffer, m_VkReadBackMemory);
}

bool MVulkanBufferPool::AllowBufferMemory(MShaderConstantParam* pParam)
{
	if (!pParam)
		return false;

	if (pParam->m_VkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		return AllowUniformBufferMemory(pParam);

	if (pParam->m_VkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
		return AllowDynamicUniformBufferMemory(pParam);

	return false;
}

bool MVulkanBufferPool::AllowUniformBufferMemory(MShaderConstantParam* pParam)
{
	uint32_t unSize = pParam->var.GetSize();

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{

		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;

		if (!m_pDevice->m_ObjectDestructor.GenerateBuffer(unSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
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

bool MVulkanBufferPool::AllowDynamicUniformBufferMemory(MShaderConstantParam* pParam)
{
	if (pParam->m_VkBuffer[0] != VK_NULL_HANDLE)
		return false;

	uint32_t unVariantSize = pParam->var.GetSize();

	if (unVariantSize > 0) {
		unVariantSize = (unVariantSize + m_unMinUboAlignment - 1) & ~(m_unMinUboAlignment - 1);
	}

	MemoryInfo allowInfo[M_BUFFER_NUM];

	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		if (!m_DynamicUniformMemoryPool.AllowMemory(unVariantSize, allowInfo[i]))
		{
			for (uint32_t n = 0; n < i; ++n)
				m_DynamicUniformMemoryPool.FreeMemory(allowInfo[n]);
			
			return false;
		}
	}

	auto& aMemoryInfo = m_tDynamicUniformMemory[pParam];
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		aMemoryInfo[i] = allowInfo[i];

		pParam->m_VkBuffer[i] = m_VkDynamicUniformBuffer;
		pParam->m_VkBufferMemory[i] = m_VkDynamicUniformMemory;
		pParam->m_unMemoryOffset[i] = allowInfo[i].begin + unVariantSize * i;
		pParam->m_pMemoryMapping[i] = m_pDynamicUniformMemoryMapping;
		pParam->m_unVkMemorySize = unVariantSize;
	}

	return true;
}

void MVulkanBufferPool::FreeBufferMemory(MShaderConstantParam* pParam)
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

bool MVulkanBufferPool::AllowReadBackBuffer(const uint32_t& unMemorySize, uint32_t& unBufferID, MemoryInfo& info)
{
	if (m_ReadBackMemoryPool.AllowMemory(unMemorySize, info))
	{
		unBufferID = m_ReadBackIDPool.GetNewID();
		m_tReadBackMemory[unBufferID] = info;
		return true;
	}

	return false;
}

void MVulkanBufferPool::FreeReadBackBuffer(const uint32_t& unBufferID)
{
	auto findResult = m_tReadBackMemory.find(unBufferID);
	if (findResult != m_tReadBackMemory.end())
	{
		m_ReadBackMemoryPool.FreeMemory(findResult->second);
		m_ReadBackIDPool.RecoveryID(unBufferID);
	}
}

void MVulkanBufferPool::FreeUniformBufferMemory(MShaderConstantParam* pParam)
{
	for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
	{
		vkUnmapMemory(m_pDevice->m_VkDevice, pParam->m_VkBufferMemory[i]);
		pParam->m_pMemoryMapping[i] = nullptr;

		m_pDevice->m_ObjectDestructor.DestroyBufferLater(pParam->m_VkBuffer[i]);
		m_pDevice->m_ObjectDestructor.DestroyDeviceMemoryLater(pParam->m_VkBufferMemory[i]);

		pParam->m_VkBuffer[i] = VK_NULL_HANDLE;
		pParam->m_VkBufferMemory[i] = VK_NULL_HANDLE;
		pParam->m_unMemoryOffset[i] = 0;
	}
}

void MVulkanBufferPool::FreeDynamicUniformBufferMemory(MShaderConstantParam* pParam)
{
	auto findResult = m_tDynamicUniformMemory.find(pParam);
	if (findResult != m_tDynamicUniformMemory.end())
	{
		for (uint32_t i = 0; i < M_BUFFER_NUM; ++i)
		{
			MemoryInfo info = findResult->second[i];
			m_DynamicUniformMemoryPool.FreeMemory(info);
		}
		m_tDynamicUniformMemory.erase(pParam);
	}
}
