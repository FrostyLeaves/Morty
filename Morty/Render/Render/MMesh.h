/**
 * @File         MMesh
 * 
 * @Created      2019-08-06 17:29:47
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Math/Vector.h"
#include <vector>

#include "MBuffer.h"

class MIDevice;
class MBuffer;
struct MVertex;
class MMaterial;


struct MMeshDrawable
{
#if RENDER_GRAPHICS == MORTY_VULKAN
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
#endif
	uint32_t nBeginOffset = 0;
	uint32_t nSize = 0;
};

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
	MByte* GetVertices() { return m_vVertexData.data(); }
	const void* GetVertices() const { return m_vVertexData.data(); }
	const std::vector<MByte>& GetVerticesVector() const { return m_vVertexData; }
	const std::vector<MByte>& GetIndicesVector() const { return m_vIndexData; }
	virtual const uint32_t* GetIndices() const { return reinterpret_cast<const uint32_t*>(m_vIndexData.data()); }
	virtual uint32_t* GetIndices() { return reinterpret_cast<uint32_t*>(m_vIndexData.data()); }

	virtual uint32_t GetVertexStructSize() const = 0;
    uint32_t GetIndexStructSize() const { return sizeof(uint32_t); }

	virtual uint32_t GetVerticesNum() const;
	virtual uint32_t GetIndicesNum() const;

	uint32_t GetVerticesSize() const { return static_cast<uint32_t>(m_vertexBuffer.GetSize()); }
	uint32_t GetIndicesSize() const { return static_cast<uint32_t>(m_indexBuffer.GetSize()); }

	virtual MIMesh* Clone(const bool& bDynamic = false) const = 0;


public:

	virtual void CreateVertices(const uint32_t& nSize) = 0;
	virtual void ResizeVertices(const uint32_t& nSize) = 0;

	virtual void CreateIndices(const uint32_t& nSize, const uint32_t& nIndexSize);
	virtual void ResizeIndices(const uint32_t& nSize, const uint32_t& nIndexSize);

	virtual void Clean()
	{
		ResizeVertices(0);
		ResizeIndices(0, 1);
	}

protected:
	MBuffer m_vertexBuffer;
	MBuffer m_indexBuffer;

	std::vector<MByte> m_vVertexData;
	std::vector<MByte> m_vIndexData;

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

	virtual void CreateVertices(const uint32_t& nSize) override
	{
		if (m_vertexBuffer.GetSize() < nSize * sizeof(VERTEX_TYPE))
		{
			m_vertexBuffer.ReallocMemory(nSize * sizeof(VERTEX_TYPE));
			m_vVertexData.resize(nSize * sizeof(VERTEX_TYPE));
		}
	}

	virtual void ResizeVertices(const uint32_t& nSize) override
	{
		if (m_vertexBuffer.GetSize() < nSize * sizeof(VERTEX_TYPE))
		{
			m_vertexBuffer.ReallocMemory(nSize * sizeof(VERTEX_TYPE));
			m_vVertexData.resize(nSize * sizeof(VERTEX_TYPE));
		}
	}

	VERTEX_TYPE* GetVertices()
	{
		return reinterpret_cast<VERTEX_TYPE*>(m_vVertexData.data());
	}
};
