﻿#include "MMesh.h"
#include "MIDevice.h"
#include "MVertex.h"

MIMesh::MIMesh(const bool& bModifiable/* = false*/)
	: m_vVertices(nullptr)
	, m_vIndices(nullptr)
	, m_unVerticesLength(0)
	, m_unIndicesLength(0)
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

void MIMesh::CreateIndices(const uint32_t& unSize, const uint32_t& unIndexSize)
{
	if (m_unIndicesArraySize < unSize * unIndexSize)
	{
		if (nullptr != m_vIndices)
		{
			delete[] m_vIndices;
			m_vIndices = nullptr;
		}

		m_vIndices = new uint32_t[unSize * unIndexSize];
		m_unIndicesArraySize = unSize * unIndexSize;

		m_bNeedGenerate = true;
	}

	m_unIndicesLength = unSize * unIndexSize;
}

void MIMesh::ResizeIndices(const uint32_t& unSize, const uint32_t& unIndexSize)
{
	if (m_unIndicesArraySize < unSize * unIndexSize)
	{
		uint32_t* indices = new uint32_t[unSize * unIndexSize];
		
		if (nullptr != m_vIndices)
		{
			memcpy(indices, m_vIndices, m_unIndicesLength * sizeof(uint32_t));
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
	if (m_bModifiable && m_pVertexBuffer)
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
