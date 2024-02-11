#include "Render/MMesh.h"

#include "MBuffer.h"
#include "MBuffer.h"
#include "Render/MIDevice.h"
#include "Render/MVertex.h"

using namespace morty;

MIMesh::MIMesh(const bool& bDynamicMesh/* = false*/)
	: m_vertexBuffer()
	, m_indexBuffer()
{
    if (bDynamicMesh)
    {
		m_vertexBuffer = MBuffer::CreateHostVisibleVertexBuffer("Mesh VertexBuffer");
		m_indexBuffer = MBuffer::CreateHostVisibleIndexBuffer("Mesh IndexBuffer");
    }
    else
    {
		m_vertexBuffer = MBuffer::CreateVertexBuffer("Mesh VertexBuffer");
		m_indexBuffer = MBuffer::CreateIndexBuffer("Mesh IndexBuffer");
    }

}

MIMesh::~MIMesh()
{
}

uint32_t MIMesh::GetVerticesNum() const
{
	return static_cast<uint32_t>(m_vertexBuffer.GetSize() / GetVertexStructSize());
}

uint32_t MIMesh::GetIndicesNum() const
{
	return static_cast<uint32_t>(m_indexBuffer.GetSize() / sizeof(uint32_t));
}

void MIMesh::CreateIndices(const uint32_t& unSize, const uint32_t& unIndexSize)
{
	if (m_indexBuffer.GetSize() < unSize * unIndexSize * sizeof(uint32_t))
	{
		m_indexBuffer.ReallocMemory(unSize * unIndexSize * sizeof(uint32_t));
		m_vIndexData.resize(unSize * unIndexSize * sizeof(uint32_t));
	}
}

void MIMesh::ResizeIndices(const uint32_t& unSize, const uint32_t& unIndexSize)
{
	if (m_indexBuffer.GetSize() < unSize * unIndexSize * sizeof(uint32_t))
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
