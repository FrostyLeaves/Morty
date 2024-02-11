#pragma once

#include "Object/MObject.h"
#include "Render/MMesh.h"
#include "Render/MBuffer.h"
#include "Utility/MGlobal.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"

MORTY_SPACE_BEGIN

class MTaskNode;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MMeshBufferAdapter;
class MRenderMeshComponent;

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
	
	const MMeshData& GetCubeMesh() const;

public:
	const MBuffer* GetVertexBuffer() const { return &m_vertexBuffer; }
	const MBuffer* GetIndexBuffer() const { return &m_indexBuffer; }

	std::shared_ptr<MMeshBufferAdapter> GetMeshBuffer() const;

private:

	void InitializeScreenRect();
	void ReleaseScreenRect();
	void InitializeSkyBox();
	void ReleaseSkyBox();
	void InitializeCube();
	void ReleaseCube();

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
	std::unique_ptr<MIMesh> m_pCubeMesh = nullptr;


// render thread.
	std::mutex m_uploadMutex;
	std::vector<MIMesh*> m_vUploadQueue;
	std::shared_ptr<MMeshBufferAdapter> m_pMeshBufferAdapter = nullptr;
};

MORTY_SPACE_END