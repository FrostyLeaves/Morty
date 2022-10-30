#include "MBuffer.h"
#include "Render/MIDevice.h"



MBuffer::MBuffer()
{
	
}

MBuffer::~MBuffer()
{
	
}

const MBuffer& MBuffer::operator=(const MBuffer& other)
{
#if _DEBUG
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

void MBuffer::GenerateBuffer(MIDevice* pDevice, const std::vector<MByte>& initialData)
{
	pDevice->GenerateBuffer(this, initialData);
}

void MBuffer::UploadBuffer(MIDevice* pDevice, const std::vector<MByte>& data)
{
	pDevice->UploadBuffer(this, data);
}

void MBuffer::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyBuffer(this);
}