/**
 * @File         MMesh
 * 
 * @Created      2019-08-06 17:29:47
 *
 * @Author       DoubleYe
**/

#ifndef _M_MMESH_H_
#define _M_MMESH_H_
#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include <vector>

#include "MBuffer.h"

class MIDevice;
class MBuffer;
struct MVertex;
class MMaterial;

class MORTY_API MIMesh
{
public:
	MIMesh(const bool& nDynamicMesh = false);
	virtual ~MIMesh();

	MBuffer* GetVertexBuffer() { return &m_vertexBuffer; }
	MBuffer* GetIndexBuffer() { return &m_indexBuffer; }

	void SetDirty();
	virtual void GenerateBuffer(MIDevice* pDevice);
	virtual void UploadBuffer(MIDevice* pDevice);
	virtual void DestroyBuffer(MIDevice* pDevice);
	void* GetVertices() { return m_vertexBuffer.GetData(); }
	const void* GetVertices() const { return m_vertexBuffer.GetData(); }
	virtual const uint32_t* GetIndices() const { return reinterpret_cast<const uint32_t*>(m_indexBuffer.GetData()); }
	virtual uint32_t* GetIndices() { return reinterpret_cast<uint32_t*>(m_indexBuffer.GetData()); }

	virtual uint32_t GetVertexStructSize() const = 0;

	virtual uint32_t GetVerticesNum() const;
	virtual uint32_t GetIndicesNum() const;

	uint32_t GetVerticesSize() const { return m_vertexBuffer.GetSize(); }

	virtual MIMesh* Clone(const bool& bDynamic = false) const = 0;


public:

	virtual void CreateVertices(const uint32_t& unSize) = 0;
	virtual void ResizeVertices(const uint32_t& unSize) = 0;

	virtual void CreateIndices(const uint32_t& unSize, const uint32_t& unIndexSize);
	virtual void ResizeIndices(const uint32_t& unSize, const uint32_t& unIndexSize);

	virtual void Clean()
	{
		ResizeVertices(0);
		ResizeIndices(0, 1);
	}

protected:
	MBuffer m_vertexBuffer;
	MBuffer m_indexBuffer;

};

template <class VERTEX_TYPE>
class MORTY_API MMesh : public MIMesh
{
public:
	MMesh(const bool& nDynamicMesh = false)
		: MIMesh(nDynamicMesh){}
	virtual ~MMesh(){}

	virtual MIMesh* Clone(const bool& bDynamic = false) const override
	{
		MMesh<VERTEX_TYPE>* pNewMesh = new MMesh<VERTEX_TYPE>(bDynamic);
		pNewMesh->m_vertexBuffer = m_vertexBuffer;
		pNewMesh->m_indexBuffer = m_indexBuffer;

		return pNewMesh;
	}
public:

	virtual uint32_t GetVertexStructSize() const override
	{
		return sizeof(VERTEX_TYPE);
	}

	virtual void CreateVertices(const uint32_t& unSize) override
	{
		if (m_vertexBuffer.GetSize() < unSize * sizeof(VERTEX_TYPE))
		{
			m_vertexBuffer.ReallocMemory(unSize * sizeof(VERTEX_TYPE));
		}
	}

	virtual void ResizeVertices(const uint32_t& unSize) override
	{
		if (m_vertexBuffer.GetSize() < unSize * sizeof(VERTEX_TYPE))
		{
			m_vertexBuffer.ReallocMemory(unSize * sizeof(VERTEX_TYPE));
		}
	}

	VERTEX_TYPE* GetVertices()
	{
		return reinterpret_cast<VERTEX_TYPE*>(m_vertexBuffer.GetData());
	}
};

#endif
