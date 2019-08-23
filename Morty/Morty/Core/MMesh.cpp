#include "MMesh.h"
#include "MIRenderer.h"

MMesh::MMesh()
	: m_vVertices(nullptr)
	, m_vIndices(nullptr)
	, m_pVertexBuffer(nullptr)
{
    
}

MMesh::~MMesh()
{
	if (nullptr != m_vVertices)
		delete[] m_vVertices;
	if (nullptr != m_vIndices)
		delete[] m_vIndices;
}

void MMesh::CreateVertices(const unsigned int& unSize)
{
	if (nullptr != m_vVertices)
	{
		delete[] m_vVertices;
		m_vVertices = nullptr;
	}

	if (unSize > 0)
	{
		m_vVertices = new Vertex[unSize];
	}
}

void MMesh::CreateIndices(const unsigned int& unSize, const unsigned int& unIndexSize)
{
	if (nullptr != m_vIndices)
	{
		delete[] m_vIndices;
		m_vIndices = nullptr;
	}

	if (unSize * unIndexSize > 0)
	{
		m_vIndices = new unsigned int[unSize * unIndexSize];
	}

}

void MMesh::GenerateBuffer(MIRenderer* pRenderer)
{
	if (m_pVertexBuffer)
		pRenderer->DestroyBuffer(&m_pVertexBuffer);

	pRenderer->GenerateBuffer(&m_pVertexBuffer, this);
}
