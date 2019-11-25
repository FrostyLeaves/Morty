#include "MMesh.h"
#include "MIDevice.h"
#include "MVertex.h"

MIMesh::MIMesh(const bool& bModifiable/* = false*/)
	: m_vVertices(nullptr)
	, m_vIndices(nullptr)
	, m_pVertexBuffer(nullptr)
	, m_bNeedGenerate(true)
	, m_bNeedUpload(false)
	, m_bModifiable(bModifiable)
	, m_unVerticesArraySize(0)
	, m_unIndicesArraySize(0)
{
    
}

MIMesh::~MIMesh()
{
	if (nullptr != m_vVertices)
		delete[] m_vVertices;
	if (nullptr != m_vIndices)
		delete[] m_vIndices;
}

void MIMesh::CreateIndices(const unsigned int& unSize, const unsigned int& unIndexSize)
{
	if (m_unIndicesArraySize < unSize * unIndexSize)
	{
		if (nullptr != m_vIndices)
		{
			delete[] m_vIndices;
			m_vIndices = nullptr;
		}

		m_vIndices = new unsigned int[unSize * unIndexSize];
		m_unIndicesArraySize = unSize * unIndexSize;

		m_bNeedGenerate = true;
	}

	m_unIndicesLength = unSize * unIndexSize;
}

void MIMesh::ResizeIndices(const unsigned int& unSize, const unsigned int& unIndexSize)
{
	if (m_unIndicesArraySize < unSize * unIndexSize)
	{
		unsigned int* indices = new unsigned int[unSize * unIndexSize];
		
		if (nullptr != m_vIndices)
		{
			memcpy(indices, m_vIndices, m_unIndicesLength * sizeof(unsigned int));
			delete[] m_vIndices;
		}

		m_vIndices = indices;
		m_unIndicesArraySize = unSize * unIndexSize;

		m_bNeedGenerate = true;
	}

	m_unIndicesLength = unSize * unIndexSize;
}

void MIMesh::GenerateBuffer(MIDevice* pDevice)
{
	if (m_pVertexBuffer)
		pDevice->DestroyBuffer(&m_pVertexBuffer);

	pDevice->GenerateBuffer(&m_pVertexBuffer, this, m_bModifiable);
	m_bNeedGenerate = false;
}

void MIMesh::UploadBuffer(MIDevice* pDevice)
{
	if (m_bModifiable)
		pDevice->UploadBuffer(&m_pVertexBuffer, this);
	else
		GenerateBuffer(pDevice);

	m_bNeedUpload = false;
}

void MIMesh::DestroyBuffer(MIDevice* pDevice)
{
	if (m_pVertexBuffer)
		pDevice->DestroyBuffer(&m_pVertexBuffer);
}
