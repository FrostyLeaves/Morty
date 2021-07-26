/**
 * @File         MMesh
 * 
 * @Created      2019-08-06 17:29:47
 *
 * @Author       DoubleYe
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

class MORTY_API MIMesh
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
	virtual uint32_t GetVerticesLength() const { return m_unVerticesLength; }
	virtual uint32_t GetIndicesLength()  const { return m_unIndicesLength; }
	void* GetVertices() { return m_vVertices; }
	const void* GetVertices() const { return m_vVertices; }
	virtual const uint32_t* GetIndices() const { return m_vIndices; }
	virtual uint32_t* GetIndices() { return m_vIndices; }

	virtual uint32_t GetVertexStructSize() const = 0;

	virtual MIMesh* Copy(const bool& bModifiable = false) const = 0;

public:

	virtual void CreateVertices(const uint32_t& unSize) = 0;
	virtual void CreateIndices(const uint32_t& unSize, const uint32_t& unIndexSize);
	virtual void ResizeVertices(const uint32_t& unSize) = 0;
	virtual void ResizeIndices(const uint32_t& unSize, const uint32_t& unIndexSize);

	virtual void Clean()
	{
		ResizeVertices(0);
		ResizeIndices(0, 1);
	}

protected:

	void* m_vVertices;
	uint32_t* m_vIndices;
	uint32_t m_unVerticesLength;
	uint32_t m_unIndicesLength;

	uint32_t m_unVerticesArraySize;
	uint32_t m_unIndicesArraySize;

	MVertexBuffer* m_pVertexBuffer;

	bool m_bNeedGenerate;
	bool m_bNeedUpload;
	bool m_bModifiable;
};

template <class VERTEX_TYPE>
class MORTY_API MMesh : public MIMesh
{
public:
	MMesh(const bool& bModifiable = false)
		: MIMesh(bModifiable){}
	virtual ~MMesh(){}

	virtual MIMesh* Copy(const bool& bModifiable = false) const override
	{
		MMesh<VERTEX_TYPE>* pNewMesh = new MMesh<VERTEX_TYPE>(bModifiable);
		pNewMesh->ResizeIndices(GetIndicesLength(), 1);
		pNewMesh->ResizeVertices(GetVerticesLength());

		memcpy(pNewMesh->m_vIndices, m_vIndices, m_unIndicesLength * sizeof(uint32_t));
		memcpy(pNewMesh->m_vVertices, m_vVertices, m_unVerticesLength * sizeof(VERTEX_TYPE));

		return pNewMesh;
	}
public:

	virtual uint32_t GetVertexStructSize() const override
	{
		return sizeof(VERTEX_TYPE);
	}

	virtual void CreateVertices(const uint32_t& unSize) override
	{
		if (m_unVerticesArraySize < unSize)
		{
			if (nullptr != m_vVertices)
			{
				delete[] (VERTEX_TYPE*)m_vVertices;
				m_vVertices = nullptr;
			}

			m_vVertices = new VERTEX_TYPE[unSize];
			m_unVerticesArraySize = unSize;

			m_bNeedGenerate = true;
		}

		m_unVerticesLength = unSize;
	}

	virtual void ResizeVertices(const uint32_t& unSize) override
	{
		if (m_unVerticesArraySize < unSize)
		{
			VERTEX_TYPE* vertices = new VERTEX_TYPE[unSize];
			if (nullptr != m_vVertices)
			{
				memcpy(vertices, m_vVertices, m_unVerticesLength * sizeof(VERTEX_TYPE));
				delete[](VERTEX_TYPE*)m_vVertices;
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
