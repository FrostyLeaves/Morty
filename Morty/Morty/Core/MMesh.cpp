#include "MMesh.h"
#include "MIRenderer.h"
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

void MIMesh::GenerateBuffer(MIRenderer* pRenderer)
{
	if (m_pVertexBuffer)
		pRenderer->DestroyBuffer(&m_pVertexBuffer);

	pRenderer->GenerateBuffer(&m_pVertexBuffer, this, m_bModifiable);
	m_bNeedGenerate = false;
}

void MIMesh::UploadBuffer(MIRenderer* pRenderer)
{
	pRenderer->UploadBuffer(&m_pVertexBuffer, this);

	m_bNeedUpload = false;
}
