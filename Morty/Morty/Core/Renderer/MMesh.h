/**
 * @File         MMesh
 * 
 * @Created      2019-08-06 17:29:47
 *
 * @Author       Pobrecito
**/

#ifndef _M_MMESH_H_
#define _M_MMESH_H_
#include "MGlobal.h"
#include "Vector.h"
#include <vector>

class MIDevice;
class MVertexBuffer;
struct MVertex;
class MMaterial;

class MORTY_CLASS MIMesh
{
public:
	MIMesh(const bool& bModifiable = false);
	virtual ~MIMesh();

	virtual MVertexBuffer* GetBuffer() { return m_pVertexBuffer; }

	virtual bool GetNeedGenerate(){ return nullptr == m_pVertexBuffer || m_bNeedGenerate; }
	virtual bool GetNeedUpload() { return m_bNeedUpload; }
	virtual void SetNeedUpload(){ m_bNeedUpload = true; }

	virtual void GenerateBuffer(MIDevice* pDevice);
	virtual void UploadBuffer(MIDevice* pDevice);
	virtual void DestroyBuffer(MIDevice* pDevice);
	virtual unsigned int GetVerticesLength() { return m_unVerticesLength; }
	virtual unsigned int GetIndicesLength() { return m_unIndicesLength; }
	void* GetVertices() { return m_vVertices; }
	virtual unsigned int* GetIndices() { return m_vIndices; }

	virtual unsigned int GetVertexStructSize() = 0;

public:

	virtual void CreateVertices(const unsigned int& unSize) = 0;
	virtual void CreateIndices(const unsigned int& unSize, const unsigned int& unIndexSize);
	virtual void ResizeVertices(const unsigned int& unSize) = 0;
	virtual void ResizeIndices(const unsigned int& unSize, const unsigned int& unIndexSize);

	virtual void Clean()
	{
		ResizeVertices(0);
		ResizeIndices(0, 1);
	}

protected:

	void* m_vVertices;
	unsigned int* m_vIndices;
	unsigned int m_unVerticesLength;
	unsigned int m_unIndicesLength;

	unsigned int m_unVerticesArraySize;
	unsigned int m_unIndicesArraySize;

	MVertexBuffer* m_pVertexBuffer;

	bool m_bNeedGenerate;
	bool m_bNeedUpload;
	bool m_bModifiable;
};

template <class VERTEX_TYPE>
class MORTY_CLASS MMesh : public MIMesh
{
public:
	MMesh(const bool& bModifiable = false)
		: MIMesh(bModifiable){}
	virtual ~MMesh(){}

public:

	virtual unsigned int GetVertexStructSize() override
	{
		return sizeof(VERTEX_TYPE);
	}

	virtual void CreateVertices(const unsigned int& unSize) override
	{
		if (m_unVerticesArraySize < unSize)
		{
			if (nullptr != m_vVertices)
			{
				delete[] m_vVertices;
				m_vVertices = nullptr;
			}

			m_vVertices = new VERTEX_TYPE[unSize];
			m_unVerticesArraySize = unSize;

			m_bNeedGenerate = true;
		}

		m_unVerticesLength = unSize;
	}

	virtual void ResizeVertices(const unsigned int& unSize) override
	{
		if (m_unVerticesArraySize < unSize)
		{
			VERTEX_TYPE* vertices = new VERTEX_TYPE[unSize];
			if (nullptr != m_vVertices)
			{
				memcpy(vertices, m_vVertices, m_unVerticesLength * sizeof(VERTEX_TYPE));
				delete[] m_vVertices;
			}

			m_vVertices = vertices;
			m_unVerticesArraySize = unSize;

			m_bNeedGenerate = true;
		}

		m_unVerticesLength = unSize;
	}

	VERTEX_TYPE* GetVertices()
	{
		return static_cast<VERTEX_TYPE*>(m_vVertices);
	}
};

#endif
