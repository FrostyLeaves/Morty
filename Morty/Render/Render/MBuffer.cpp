#include "MBuffer.h"
#include "Render/MIDevice.h"



MBuffer::MBuffer()
{
	
}

MBuffer::~MBuffer()
{
	
}


MBuffer MBuffer::CreateVertexBuffer(const char* debugName)
{
	MBuffer buffer;
	buffer.m_eUsageType = MUsageType::EVertex;
	buffer.m_eMemoryType = MMemoryType::EDeviceLocal;
#if MORTY_DEBUG
	buffer.m_strDebugBufferName = debugName;
#endif
	return buffer;
}

MBuffer MBuffer::CreateHostVisibleVertexBuffer(const char* debugName)
{
	MBuffer buffer;
	buffer.m_eUsageType = MUsageType::EVertex;
	buffer.m_eMemoryType = MMemoryType::EHostVisible;
#if MORTY_DEBUG
	buffer.m_strDebugBufferName = debugName;
#endif
	return buffer;
}

MBuffer MBuffer::CreateIndexBuffer(const char* debugName)
{
	MBuffer buffer;
	buffer.m_eUsageType = MUsageType::EIndex;
	buffer.m_eMemoryType = MMemoryType::EDeviceLocal;
#if MORTY_DEBUG
	buffer.m_strDebugBufferName = debugName;
#endif
	return buffer;
}

MBuffer MBuffer::CreateHostVisibleIndexBuffer(const char* debugName)
{
	MBuffer buffer;
	buffer.m_eUsageType = MUsageType::EIndex;
	buffer.m_eMemoryType = MMemoryType::EHostVisible;
#if MORTY_DEBUG
	buffer.m_strDebugBufferName = debugName;
#endif
	return buffer;
}

MBuffer MBuffer::CreateIndirectDrawBuffer(const char* debugName)
{
	MBuffer buffer;
	buffer.m_eUsageType = MBuffer::MUsageType::EStorage | MBuffer::MUsageType::EIndirect;
	buffer.m_eMemoryType = MBuffer::MMemoryType::EDeviceLocal;
#if MORTY_DEBUG
	buffer.m_strDebugBufferName = debugName;
#endif
	return buffer;
}

MBuffer MBuffer::CreateStorageBuffer(const char* debugName)
{
	MBuffer buffer;
	buffer.m_eUsageType = MBuffer::MUsageType::EStorage;
	buffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
#if MORTY_DEBUG
	buffer.m_strDebugBufferName = debugName;
#endif
	return buffer;
}

const MBuffer& MBuffer::operator=(const MBuffer& other)
{	
	m_eMemoryType = other.m_eMemoryType;
	m_eUsageType = other.m_eUsageType;

	m_eStageType = MStageType::EWaitAllow;

	return *this;
}

void MBuffer::ReallocMemory(const size_t& unNewSize)
{
	m_unDataSize = unNewSize;
	m_eStageType = MBuffer::MStageType::EWaitAllow;
}

void MBuffer::GenerateBuffer(MIDevice* pDevice, const MByte* data, const size_t& size)
{
	pDevice->GenerateBuffer(this, data, size);
}

void MBuffer::UploadBuffer(MIDevice* pDevice, const MByte* data, const size_t& size)
{
	pDevice->UploadBuffer(this, 0, data, size);
}

void MBuffer::UploadBuffer(MIDevice* pDevice, size_t nBeginOffset, const MByte* data, const size_t& size)
{
	pDevice->UploadBuffer(this, nBeginOffset, data, size);
}

void MBuffer::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyBuffer(this);
}

void MBuffer::DownloadBuffer(MIDevice* pDevice, MByte* data, const size_t& size)
{
	pDevice->DownloadBuffer(this, data, size);
}
