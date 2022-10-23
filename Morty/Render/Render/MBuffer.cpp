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

	m_data = other.m_data;
	m_eMemoryType = other.m_eMemoryType;
	m_eUsageType = other.m_eUsageType;

	m_eStageType = MStageType::EWaitAllow;

	return *this;
}

void MBuffer::ReallocMemory(const size_t& unNewSize)
{
	m_data.resize(unNewSize);
	m_eStageType = MBuffer::MStageType::EWaitAllow;
}

void MBuffer::GenerateBuffer(MIDevice* pDevice)
{
	pDevice->GenerateBuffer(this);
}

void MBuffer::UploadBuffer(MIDevice* pDevice)
{
	pDevice->UploadBuffer(this);
}

void MBuffer::DestroyBuffer(MIDevice* pDevice)
{
	pDevice->DestroyBuffer(this);
}