#include "Render/MMesh.h"

#include "MBuffer.h"
#include "MBuffer.h"
#include "Render/MIDevice.h"
#include "Render/MVertex.h"

MIMesh::MIMesh(const bool& bDynamicMesh/* = false*/)
	: m_vertexBuffer()
	, m_indexBuffer()
{
    if (bDynamicMesh)
    {
		m_vertexBuffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
		m_indexBuffer.m_eMemoryType = MBuffer::MMemoryType::EHostVisible;
    }
    else
    {
		m_vertexBuffer.m_eMemoryType = MBuffer::MMemoryType::EDeviceLocal;
		m_indexBuffer.m_eMemoryType = MBuffer::MMemoryType::EDeviceLocal;
    }

	m_vertexBuffer.m_eUsageType |= MBuffer::MUsageType::EVertex;
	m_indexBuffer.m_eUsageType |= MBuffer::MUsageType::EIndex;

#if _DEBUG
	m_vertexBuffer.m_strDebugBufferName = "VertexBuffer";
	m_indexBuffer.m_strDebugBufferName = "IndexBuffer";
#endif
}

MIMesh::~MIMesh()
{
}

uint32_t MIMesh::GetVerticesNum() const
{
	return m_vertexBuffer.GetSize() / GetVertexStructSize();
}

uint32_t MIMesh::GetIndicesNum() const
{
	return m_indexBuffer.GetSize() / sizeof(uint32_t);
}

void MIMesh::CreateIndices(const uint32_t& unSize, const uint32_t& unIndexSize)
{
	if (m_indexBuffer.GetSize() < unSize * unIndexSize)
	{
		m_indexBuffer.ReallocMemory(unSize * unIndexSize * sizeof(uint32_t));
		m_vIndexData.resize(unSize * unIndexSize * sizeof(uint32_t));
	}
}

void MIMesh::ResizeIndices(const uint32_t& unSize, const uint32_t& unIndexSize)
{
	if (m_indexBuffer.GetSize() < unSize * unIndexSize)
	{
		m_indexBuffer.ReallocMemory(unSize * unIndexSize * sizeof(uint32_t));
		m_vIndexData.resize(unSize * unIndexSize * sizeof(uint32_t));
	}
}

void MIMesh::SetDirty()
{
	if (m_vertexBuffer.m_eStageType == MBuffer::MStageType::ESynced)
	{
		m_vertexBuffer.m_eStageType = MBuffer::MStageType::EWaitSync;
	}

	if (m_indexBuffer.m_eStageType == MBuffer::MStageType::ESynced)
	{
		m_indexBuffer.m_eStageType = MBuffer::MStageType::EWaitSync;
	}
}

void MIMesh::GenerateBuffer(MIDevice* pDevice)
{
	m_vertexBuffer.GenerateBuffer(pDevice, m_vVertexData.data(), m_vVertexData.size());
	m_indexBuffer.GenerateBuffer(pDevice, m_vIndexData.data(), m_vIndexData.size());
}

void MIMesh::UploadBuffer(MIDevice* pDevice)
{
	m_vertexBuffer.UploadBuffer(pDevice, m_vVertexData.data(), m_vVertexData.size());
	m_indexBuffer.UploadBuffer(pDevice, m_vIndexData.data(), m_vIndexData.size());
}

void MIMesh::DestroyBuffer(MIDevice* pDevice)
{
	m_vertexBuffer.DestroyBuffer(pDevice);
	m_indexBuffer.DestroyBuffer(pDevice);
}
