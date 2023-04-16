#include "MBuffer.h"
#include "Render/MIDevice.h"



MBuffer::MBuffer()
{
	
}

MBuffer::~MBuffer()
{
	
}


MBuffer MBuffer::CreateVertexBuffer()
{
	MBuffer buffer;
	buffer.m_eUsageType = MUsageType::EVertex;
	buffer.m_eMemoryType = MMemoryType::EDeviceLocal;

	return buffer;
}

MBuffer MBuffer::CreateHostVisibleVertexBuffer()
{
	MBuffer buffer;
	buffer.m_eUsageType = MUsageType::EVertex;
	buffer.m_eMemoryType = MMemoryType::EHostVisible;

	return buffer;
}

MBuffer MBuffer::CreateIndexBuffer()
{
	MBuffer buffer;
	buffer.m_eUsageType = MUsageType::EIndex;
	buffer.m_eMemoryType = MMemoryType::EDeviceLocal;

	return buffer;
}

MBuffer MBuffer::CreateHostVisibleIndexBuffer()
{
	MBuffer buffer;
	buffer.m_eUsageType = MUsageType::EIndex;
	buffer.m_eMemoryType = MMemoryType::EHostVisible;

	return buffer;
}

const MBuffer& MBuffer::operator=(const MBuffer& other)
{
#if MORTY_DEBUG
	m_strDebugBufferName = other.m_strDebugBufferName;
#endif
	
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
