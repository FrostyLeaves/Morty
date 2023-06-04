#ifndef _M_MESH_MANAGER_H_
#define _M_MESH_MANAGER_H_

#include "Object/MObject.h"
#include "Utility/MGlobal.h"
#include "Render/MBuffer.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"

class MTaskNode;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MRenderableMeshComponent;

class MMeshManager : public MObject
{
	MORTY_CLASS(MMeshManager)
public:
	explicit  MMeshManager();

	virtual void OnCreated() override;
	virtual void OnDelete() override;

public:

	struct MClusterData
	{
		MemoryInfo indexInfo;
		MBoundsSphere boundsShpere;
	};

	struct MMeshData
	{
		MemoryInfo vertexInfo; //vertex offset and size.
		MemoryInfo vertexMemoryInfo; //memory allow need byte alignment.
		MemoryInfo indexInfo; //indexInfo.offset is indexMemoryInfo.offset divide by sizeof(uint32_t)
		MemoryInfo indexMemoryInfo;
		std::vector<MClusterData> vClusterData;
	};

public:

	bool RegisterMesh(MIMesh* pMesh);
	void UnregisterMesh(MIMesh* pMesh);

	bool HasMesh(MIMesh* pMesh) const;
	const MMeshData& FindMesh(MIMesh* pMesh) const;

	MIMesh* GetScreenRect() const;
	MIMesh* GetSkyBox() const;

public:
	const MBuffer* GetVertexBuffer() const { return &m_vertexBuffer; }
	const MBuffer* GetIndexBuffer() const { return &m_indexBuffer; }

private:

	void InitializeScreenRect();
	void ReleaseScreenRect();
	void InitializeSkyBox();
	void ReleaseSkyBox();

	size_t RoundIndexSize(size_t nIndexSize);
	void UploadBuffer(MIMesh* pMesh);

	void UploadBufferTask(MTaskNode* pNode);

	const size_t MeshVertexStructSize;

	MBuffer m_vertexBuffer;
	MMemoryPool m_vertexMemoryPool;

	MBuffer m_indexBuffer;
	MMemoryPool m_indexMemoryPool;

	std::map<MIMesh*, MMeshData> m_tMeshTable;
	
	std::unique_ptr<MIMesh> m_pScreenRect = nullptr;
	std::unique_ptr<MIMesh> m_pSkyBox = nullptr;


// render thread.
	std::mutex m_uploadMutex;
	std::vector<MIMesh*> m_vUploadQueue;
};


#endif