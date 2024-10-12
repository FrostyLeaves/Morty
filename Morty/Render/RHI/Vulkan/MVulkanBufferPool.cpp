#include "MVulkanBufferPool.h"
#include "RHI/Vulkan/MVulkanDevice.h"
#include "Shader/MShaderParam.h"

using namespace morty;

const uint32_t UinformMemorySize  = 32 * 1024 * 1024;
const uint32_t ReadBackMemorySize = 32 * 1024 * 1024;

MVulkanBufferPool::MVulkanBufferPool(MVulkanDevice* pDevice)
    : m_device(pDevice)

    , m_unMinUboAlignment(0)
    , m_unDynamicUniformBufferMemorySize(UinformMemorySize)
    , m_DynamicUniformMemoryPool(UinformMemorySize)
    , m_dynamicUniformMemory()
    , m_vkDynamicUniformBuffer(VK_NULL_HANDLE)
    , m_vkDynamicUniformMemory(VK_NULL_HANDLE)
    , m_dynamicUniformMemoryMapping(nullptr)

    , m_unReadBackBufferMemorySize(ReadBackMemorySize)
    , m_ReadBackMemoryPool(ReadBackMemorySize)
    , m_ReadBackIDPool()
    , m_readBackMemory()
    , m_vkReadBackBuffer(VK_NULL_HANDLE)
    , m_vkReadBackMemory(VK_NULL_HANDLE)
    , m_readBackMemoryMapping(nullptr)
{}

MVulkanBufferPool::~MVulkanBufferPool() {}

bool MVulkanBufferPool::Initialize()
{
    m_unMinUboAlignment =
            static_cast<uint32_t>(m_device->GetPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment);

    m_device->GenerateBuffer(
            m_unDynamicUniformBufferMemorySize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_vkDynamicUniformBuffer,
            m_vkDynamicUniformMemory
    );

    void* pData = nullptr;
    vkMapMemory(m_device->m_vkDevice, m_vkDynamicUniformMemory, 0, m_unDynamicUniformBufferMemorySize, 0, &pData);
    m_dynamicUniformMemoryMapping = (MByte*) pData;


    m_device->GenerateBuffer(
            m_unReadBackBufferMemorySize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_vkReadBackBuffer,
            m_vkReadBackMemory
    );

    void* pData2 = nullptr;
    vkMapMemory(m_device->m_vkDevice, m_vkReadBackMemory, 0, m_unReadBackBufferMemorySize, 0, &pData2);
    m_readBackMemoryMapping = (MByte*) pData2;

    return true;
}

void MVulkanBufferPool::Release()
{
    vkUnmapMemory(m_device->m_vkDevice, m_vkDynamicUniformMemory);
    m_device->DestroyBuffer(m_vkDynamicUniformBuffer, m_vkDynamicUniformMemory);

    vkUnmapMemory(m_device->m_vkDevice, m_vkReadBackMemory);
    m_device->DestroyBuffer(m_vkReadBackBuffer, m_vkReadBackMemory);
}

bool MVulkanBufferPool::AllowBufferMemory(const std::shared_ptr<MShaderConstantParam>& pParam)
{
    if (!pParam) return false;

    if (pParam->m_vkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) return AllowUniformBufferMemory(pParam);

    if (pParam->m_vkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
        return AllowDynamicUniformBufferMemory(pParam);

    return false;
}

bool MVulkanBufferPool::AllowUniformBufferMemory(const std::shared_ptr<MShaderConstantParam>& pParam)
{
    uint32_t       unSize = static_cast<uint32_t>(pParam->var.GetSize());


    VkBuffer       buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    if (!m_device->GenerateBuffer(
                unSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                buffer,
                memory
        ))
    {

        FreeUniformBufferMemory(pParam);
        MORTY_ASSERT(false);
        return false;
    }

    pParam->m_vkBuffer       = buffer;
    pParam->m_vkBufferMemory = memory;
    pParam->m_unMemoryOffset = 0;
    pParam->m_unVkMemorySize = unSize;

    VkDescriptorBufferInfo& bufferInfo = pParam->m_vkBufferInfo;
    bufferInfo.buffer                  = buffer;
    bufferInfo.offset                  = 0;
    bufferInfo.range                   = unSize;

    void* pData = nullptr;
    vkMapMemory(m_device->m_vkDevice, memory, 0, unSize, 0, &pData);
    pParam->m_memoryMapping = (MByte*) pData;


    return true;
}

bool MVulkanBufferPool::AllowDynamicUniformBufferMemory(const std::shared_ptr<MShaderConstantParam>& pParam)
{
    if (pParam->m_vkBuffer != VK_NULL_HANDLE)
    {
        MORTY_ASSERT(VK_NULL_HANDLE == pParam->m_vkBuffer);
        return false;
    }
    uint32_t unVariantSize = static_cast<uint32_t>(pParam->var.GetSize());

    if (unVariantSize > 0) { unVariantSize = (unVariantSize + m_unMinUboAlignment - 1) & ~(m_unMinUboAlignment - 1); }

    MemoryInfo allowInfo;

    if (!m_DynamicUniformMemoryPool.AllowMemory(unVariantSize, allowInfo))
    {
        MORTY_ASSERT(false);
        m_DynamicUniformMemoryPool.FreeMemory(allowInfo);
        return false;
    }


    m_dynamicUniformMemory[pParam] = allowInfo;

    pParam->m_vkBuffer       = m_vkDynamicUniformBuffer;
    pParam->m_vkBufferMemory = m_vkDynamicUniformMemory;
    pParam->m_unMemoryOffset = static_cast<uint32_t>(allowInfo.begin);
    pParam->m_memoryMapping  = m_dynamicUniformMemoryMapping;
    pParam->m_unVkMemorySize = unVariantSize;


    VkDescriptorBufferInfo& bufferInfo = pParam->m_vkBufferInfo;
    bufferInfo.buffer                  = m_vkDynamicUniformBuffer;
    bufferInfo.offset =
            0;//allowInfo.begin; //The real data starting at param->m_unMemoryOffset + VkDescriptorBufferInfo::offset.
    bufferInfo.range = pParam->var.GetSize();

    return true;
}

void MVulkanBufferPool::FreeBufferMemory(const std::shared_ptr<MShaderConstantParam>& pParam)
{
    if (!pParam) return;

    if (VK_NULL_HANDLE == pParam->m_vkBuffer) return;

    if (pParam->m_vkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) return FreeUniformBufferMemory(pParam);

    if (pParam->m_vkDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
        return FreeDynamicUniformBufferMemory(pParam);
}

bool MVulkanBufferPool::AllowReadBackBuffer(const uint32_t& unMemorySize, uint32_t& unBufferID, MemoryInfo& info)
{
    if (m_ReadBackMemoryPool.AllowMemory(unMemorySize, info))
    {
        unBufferID                   = m_ReadBackIDPool.GetNewID();
        m_readBackMemory[unBufferID] = info;
        return true;
    }

    return false;
}

void MVulkanBufferPool::FreeReadBackBuffer(const uint32_t& unBufferID)
{
    auto findResult = m_readBackMemory.find(unBufferID);
    if (findResult != m_readBackMemory.end())
    {
        m_ReadBackMemoryPool.FreeMemory(findResult->second);
        m_ReadBackIDPool.RecoveryID(unBufferID);
    }
}

void   MVulkanBufferPool::FreeDynamicUniformMemory(MemoryInfo& info) { m_DynamicUniformMemoryPool.FreeMemory(info); }

size_t MVulkanBufferPool::GetDynamicUniformMemorySize() const { return m_DynamicUniformMemoryPool.GetMaxMemorySize(); }

void   MVulkanBufferPool::FreeUniformBufferMemory(const std::shared_ptr<MShaderConstantParam>& pParam)
{
    vkUnmapMemory(m_device->m_vkDevice, pParam->m_vkBufferMemory);
    pParam->m_memoryMapping = nullptr;

    m_device->GetRecycleBin()->DestroyBufferLater(pParam->m_vkBuffer);
    m_device->GetRecycleBin()->DestroyDeviceMemoryLater(pParam->m_vkBufferMemory);

    pParam->m_vkBuffer       = VK_NULL_HANDLE;
    pParam->m_vkBufferMemory = VK_NULL_HANDLE;
    pParam->m_unMemoryOffset = 0;
    pParam->m_vkBufferInfo   = {};
}

void MVulkanBufferPool::FreeDynamicUniformBufferMemory(const std::shared_ptr<MShaderConstantParam>& pParam)
{
    auto findResult = m_dynamicUniformMemory.find(pParam);
    if (findResult != m_dynamicUniformMemory.end())
    {
        MemoryInfo info = findResult->second;
        m_dynamicUniformMemory.erase(pParam);


        m_device->GetRecycleBin()->DestroyDynamicUniformMemoryLater(info);

        pParam->m_vkBuffer       = VK_NULL_HANDLE;
        pParam->m_vkBufferMemory = VK_NULL_HANDLE;
        pParam->m_unMemoryOffset = 0;
        pParam->m_vkBufferInfo   = {};
    }
}
