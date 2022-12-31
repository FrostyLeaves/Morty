#ifndef _M_MERGE_INSTANCING_MESH_H_
#define _M_MERGE_INSTANCING_MESH_H_

#include "Object/MObject.h"
#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MRenderableMeshComponent;

struct MMeshCluster
{
	size_t unBeginOffset;
};

class MMergeInstancingMesh : public MObject
{
	MORTY_INTERFACE(MMergeInstancingMesh)
public:
	explicit  MMergeInstancingMesh();

	virtual void OnCreated() override;
	virtual void OnDelete() override;

public:

	struct MClusterData
	{
		MemoryInfo memoryInfo;
		MBoundsSphere boundsShpere;
	};

	struct MMeshMergeData
	{
		MemoryInfo vertexMemoryInfo;
		std::vector<MClusterData> vIndexData;
	};

public:

	bool RegisterMesh(MIMesh* pMesh);
	void UnregisterMesh(MIMesh* pMesh);

	const MMeshMergeData& FindMesh(MIMesh* pMesh);

public:
	const MBuffer* GetVertexBuffer() const { return &m_vertexBuffer; }
	const MBuffer* GetIndexBuffer() const { return &m_indexBuffer; }

private:

	const size_t MeshVertexStructSize;
	
	static constexpr size_t ClusterSize = 64u * 3u;

	MBuffer m_vertexBuffer;
	MMemoryPool m_vertexMemoryPool;

	MBuffer m_indexBuffer;
	MMemoryPool m_indexMemoryPool;

	std::map<MIMesh*, MMeshMergeData> m_tMeshTable;
};


#endif