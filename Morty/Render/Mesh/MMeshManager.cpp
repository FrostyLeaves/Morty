#include "MMeshManager.h"

#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Render/MMesh.h"
#include "Render/MVertex.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"

MORTY_CLASS_IMPLEMENT(MMeshManager, MObject)

constexpr size_t VertexMemoryMaxSize = 1024 * 1024 * 20;
constexpr size_t IndexMemoryMaxSize = 1024 * 1024 * 80;
constexpr size_t ClusterSize = 64u * 3u;

constexpr size_t VertexAllocByteAlignment = 32;

MMeshManager::MMeshManager()
	: MeshVertexStructSize(sizeof(MVertex))
	, m_vertexMemoryPool(VertexMemoryMaxSize * MeshVertexStructSize)
	, m_indexMemoryPool(IndexMemoryMaxSize * sizeof(uint32_t))
{
}

void MMeshManager::OnCreated()
{
	Super::OnCreated();

	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_vertexBuffer = MBuffer::CreateVertexBuffer();
	m_vertexBuffer.ReallocMemory(m_vertexMemoryPool.GetMaxMemorySize());
	m_vertexBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, 0);

	m_indexBuffer = MBuffer::CreateIndexBuffer();
	m_indexBuffer.ReallocMemory(m_indexMemoryPool.GetMaxMemorySize());
	m_indexBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, 0);

	InitializeScreenRect();
	InitializeSkyBox();

	MTaskNode* pUploadBufferTask = GetEngine()->GetMainGraph()->AddNode<MTaskNode>("Upload Mesh Buffer");
	pUploadBufferTask->SetThreadType(METhreadType::ERenderThread);
	pUploadBufferTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MMeshManager::UploadBufferTask, this));
}

void MMeshManager::OnDelete()
{
	ReleaseSkyBox();
	ReleaseScreenRect();

	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_vertexBuffer.DestroyBuffer(pRenderSystem->GetDevice());
	m_indexBuffer.DestroyBuffer(pRenderSystem->GetDevice());


	Super::OnDelete();
}

void MMeshManager::InitializeScreenRect()
{
	m_pScreenRect = std::make_unique<MMesh<Vector2>>(true);
	m_pScreenRect->ResizeVertices(4);
	Vector2* vVertices = (Vector2*)m_pScreenRect->GetVertices();

	vVertices[0] = Vector2(-1, -1);
	vVertices[1] = Vector2(1, -1);
	vVertices[2] = Vector2(-1, 1);
	vVertices[3] = Vector2(1, 1);

	m_pScreenRect->ResizeIndices(2, 3);
	uint32_t* vIndices = m_pScreenRect->GetIndices();

	vIndices[0] = 0;
	vIndices[1] = 2;
	vIndices[2] = 1;

	vIndices[3] = 2;
	vIndices[4] = 3;
	vIndices[5] = 1;


	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_pScreenRect->GenerateBuffer(pRenderSystem->GetDevice());
}

void MMeshManager::ReleaseScreenRect()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_pScreenRect->DestroyBuffer(pRenderSystem->GetDevice());
	m_pScreenRect = nullptr;
}

void MMeshManager::InitializeSkyBox()
{
	m_pSkyBox = std::make_unique<MMesh<Vector3>>(true);
	m_pSkyBox->ResizeVertices(8);
	m_pSkyBox->ResizeIndices(12, 3);

	Vector3* vVertices = (Vector3*)m_pSkyBox->GetVertices();
	vVertices[0] = Vector3(-1.0, -1.0, 1.0);
	vVertices[1] = Vector3(-1.0, 1.0, 1.0);
	vVertices[2] = Vector3(1.0, 1.0, 1.0);
	vVertices[3] = Vector3(1.0, -1.0, 1.0);
	vVertices[4] = Vector3(-1.0, -1.0, -1.0);
	vVertices[5] = Vector3(-1.0, 1.0, -1.0);
	vVertices[6] = Vector3(1.0, 1.0, -1.0);
	vVertices[7] = Vector3(1.0, -1.0, -1.0);

	const uint32_t indices[] = {
		3, 2, 6, 3, 6, 7,//right
		0, 1, 5, 0, 5, 4,//left
		5, 1, 2, 5, 2, 6,//top
		4, 0, 3, 4, 3, 7,//bottom
		0, 1, 2, 0, 2, 3,//front
		4, 5, 6, 4, 6, 7,//back
	};

	memcpy(m_pSkyBox->GetIndices(), indices, sizeof(indices));
}

void MMeshManager::ReleaseSkyBox()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	m_pSkyBox->DestroyBuffer(pRenderSystem->GetDevice());
	m_pSkyBox = nullptr;
}

size_t MMeshManager::RoundIndexSize(size_t unIndexNum)
{
	return size_t((unIndexNum / ClusterSize) + (unIndexNum % ClusterSize ? 1 : 0)) * ClusterSize;;
}

void MMeshManager::UploadBuffer(MIMesh* pMesh)
{
	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pDevice = pRenderSystem->GetDevice();
	MMeshData& meshMergeData = m_tMeshTable[pMesh];

	const MByte* vVertexData = pMesh->GetVertices();
	const MVertex* vVertex = reinterpret_cast<const MVertex*>(vVertexData);
	const uint32_t* vIndexData = pMesh->GetIndices();
	const size_t unVertexSize = pMesh->GetVerticesSize();
	const size_t unIndexNum = pMesh->GetIndicesNum();
	const size_t unVertexStructSize = pMesh->GetVertexStructSize();
	const size_t unRoundIndexNum = RoundIndexSize(unIndexNum);
	const MemoryInfo& vertexMemoryInfo = meshMergeData.vertexMemoryInfo;
	const MemoryInfo& indexMemoryInfo = meshMergeData.indexMemoryInfo;

	//redirect index to global vertex space.
	std::vector<uint32_t> vRedirectIndex(unRoundIndexNum);

	MemoryInfo indexInfo;
	indexInfo.begin = indexMemoryInfo.begin / sizeof(uint32_t);
	indexInfo.size = indexMemoryInfo.size / sizeof(uint32_t);

	MemoryInfo vertexInfo;
	if (vertexMemoryInfo.begin % unVertexStructSize)
	{
		vertexInfo.begin = vertexMemoryInfo.begin + (unVertexStructSize - vertexMemoryInfo.begin % unVertexStructSize);
	}
	else
	{
		vertexInfo.begin = vertexMemoryInfo.begin;
	}
	vertexInfo.size = unVertexSize;
	MORTY_ASSERT(vertexInfo.begin % unVertexStructSize == 0);
	const size_t nNewVertexIndexBegin = vertexInfo.begin / unVertexStructSize;


	size_t nCurrentIndex = 0;
	std::vector<MClusterData> meshClusterData;
	while (nCurrentIndex < unRoundIndexNum)
	{
		MClusterData indexMemoryData;

		//get bounding sphere.
		std::vector<Vector3> boundsVertex(ClusterSize);

		for (uint32_t nIndexInCluster = 0; nIndexInCluster < ClusterSize; ++nIndexInCluster)
		{
			const uint32_t originIndex = vIndexData[(std::min)(nCurrentIndex + nIndexInCluster, unIndexNum - 1)];
			const uint32_t globalIndex = nNewVertexIndexBegin + originIndex;
			vRedirectIndex[nCurrentIndex + nIndexInCluster] = globalIndex;
			boundsVertex[nIndexInCluster] = vVertex[originIndex].position;
		}

		indexMemoryData.indexInfo.begin = indexInfo.begin + nCurrentIndex;
		indexMemoryData.indexInfo.size = ClusterSize;

		indexMemoryData.boundsShpere.SetPoints(
			reinterpret_cast<const MByte*>(boundsVertex.data()),
			ClusterSize,
			0, sizeof(Vector3));

		meshClusterData.push_back(indexMemoryData);

		nCurrentIndex += ClusterSize;
	}

	pDevice->UploadBuffer(&m_vertexBuffer, vertexInfo.begin, vVertexData, unVertexSize);

	pDevice->UploadBuffer(
		&m_indexBuffer,
		indexMemoryInfo.begin,
		reinterpret_cast<const MByte*>(vRedirectIndex.data()),
		indexMemoryInfo.size);

	meshMergeData.vertexInfo = vertexInfo;
	meshMergeData.indexInfo = indexInfo;
	meshMergeData.vClusterData = std::move(meshClusterData);
}

void MMeshManager::UploadBufferTask(MTaskNode* pNode)
{
	if (m_vUploadQueue.empty())
	{
		return;
	}

	std::vector<MIMesh*> vUploadQueue;
	{
		std::lock_guard lock(m_uploadMutex);
		vUploadQueue.swap(m_vUploadQueue);
	}

	for (MIMesh* pMesh : vUploadQueue)
	{
		UploadBuffer(pMesh);
	}
}

bool MMeshManager::RegisterMesh(MIMesh* pMesh)
{
	if (!pMesh)
	{
		return false;
	}

	if (m_tMeshTable.find(pMesh) != m_tMeshTable.end())
	{
		return true;
	}

	const size_t unVertexSize = pMesh->GetVerticesSize();
	const size_t unIndexNum = pMesh->GetIndicesNum();
	const size_t unVertexStructSize = pMesh->GetVertexStructSize();
	const size_t unRoundIndexNum = RoundIndexSize(unIndexNum);

	size_t unAllocVertexMemory = unVertexSize;
	unAllocVertexMemory += unVertexStructSize;
	if (unAllocVertexMemory % VertexAllocByteAlignment)
	{
		unAllocVertexMemory = (unAllocVertexMemory / VertexAllocByteAlignment + 1) * VertexAllocByteAlignment;
	}


	//alloc vertex memory
	MemoryInfo vertexMemoryInfo;
	if (!m_vertexMemoryPool.AllowMemory(unAllocVertexMemory, vertexMemoryInfo))
	{
		MORTY_ASSERT(false);
		return false;
	}

	MemoryInfo indexMemoryInfo;
	if (!m_indexMemoryPool.AllowMemory(unRoundIndexNum * pMesh->GetIndexStructSize(), indexMemoryInfo))
	{
		MORTY_ASSERT(false);

		m_vertexMemoryPool.FreeMemory(vertexMemoryInfo);
		return false;
	}

	MMeshData& meshMergeData = m_tMeshTable[pMesh];
	meshMergeData.vertexMemoryInfo = vertexMemoryInfo;
	meshMergeData.indexMemoryInfo = indexMemoryInfo;

	{
		std::lock_guard lock(m_uploadMutex);
		m_vUploadQueue.push_back(pMesh);
	}

	return true;
}

void MMeshManager::UnregisterMesh(MIMesh* pMesh)
{
	if (!pMesh)
	{
		MORTY_ASSERT(pMesh);
		return;
	}

	auto findResult = m_tMeshTable.find(pMesh);
	if (findResult == m_tMeshTable.end())
	{
		MORTY_ASSERT(findResult != m_tMeshTable.end());
		return;
	}

	MMeshData& meshData = findResult->second;
	m_vertexMemoryPool.FreeMemory(meshData.vertexMemoryInfo);
	m_indexMemoryPool.FreeMemory(meshData.indexMemoryInfo);

	m_tMeshTable.erase(findResult);
}

bool MMeshManager::HasMesh(MIMesh* pMesh) const
{
	return m_tMeshTable.find(pMesh) != m_tMeshTable.end();
}

const MMeshManager::MMeshData& MMeshManager::FindMesh(MIMesh* pMesh) const
{
	auto findResult = m_tMeshTable.find(pMesh);
	if (findResult == m_tMeshTable.end())
	{
		static MMeshData InvalidData;
		return InvalidData;
	}

	return findResult->second;
}

MIMesh* MMeshManager::GetScreenRect() const
{
	return m_pScreenRect.get();
}

MIMesh* MMeshManager::GetSkyBox() const
{
	return m_pSkyBox.get();
}