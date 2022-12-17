#include "MMergeInstancingMesh.h"

#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Render/MMesh.h"
#include "Render/MVertex.h"
#include "System/MRenderSystem.h"

MORTY_INTERFACE_IMPLEMENT(MMergeInstancingMesh, MObject)

const size_t VertexMemoryMaxSize = 1024 * 1024 * 10;
const size_t IndexMemoryMaxSize = 1024 * 1024 * 100;

MMergeInstancingMesh::MMergeInstancingMesh()
	: MeshVertexStructSize(sizeof(MVertex))
	, m_vertexMemoryPool(VertexMemoryMaxSize * MeshVertexStructSize)
	, m_indexMemoryPool(IndexMemoryMaxSize * sizeof(uint32_t))
{
}


void MMergeInstancingMesh::OnCreated()
{
	Super::OnCreated();

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_vertexBuffer = MBuffer::CreateHostVisibleVertexBuffer();
	m_vertexBuffer.ReallocMemory(VertexMemoryMaxSize);
	m_vertexBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, 0);

	m_indexBuffer = MBuffer::CreateHostVisibleIndexBuffer();
	m_indexBuffer.ReallocMemory(IndexMemoryMaxSize);
	m_indexBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, 0);
}

void MMergeInstancingMesh::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	m_vertexBuffer.DestroyBuffer(pRenderSystem->GetDevice());
	m_indexBuffer.DestroyBuffer(pRenderSystem->GetDevice());


	Super::OnDelete();
}

bool MMergeInstancingMesh::RegisterMesh(MIMesh* pMesh)
{
	if (!pMesh)
	{
		return false;
	}

	if (pMesh->GetVertexStructSize() != MeshVertexStructSize)
	{
		return false;
	}

	if (m_tMeshTable.find(pMesh) != m_tMeshTable.end())
	{
		return true;
	}

	size_t unVertexSize = pMesh->GetVerticesSize();
	size_t unIndexSize = pMesh->GetIndicesSize();
	size_t unIndexNum = pMesh->GetIndicesNum();

	MByte* vVertexData = pMesh->GetVertices();
	MVertex* vVertex = reinterpret_cast<MVertex*>(vVertexData);
	uint32_t* vIndexData = pMesh->GetIndices();


	MemoryInfo vertexMemoryInfo;
	std::vector<MMeshClusterData> indexClusterData;

	if (!m_vertexMemoryPool.AllowMemory(unVertexSize, vertexMemoryInfo))
	{
		return false;
	}

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	MIDevice* pDevice = pRenderSystem->GetDevice();

	pDevice->UploadBuffer(&m_vertexBuffer, vertexMemoryInfo.begin, vVertexData, unVertexSize);


	size_t unVertexBeginIndex = vertexMemoryInfo.begin / MeshVertexStructSize;
	size_t nMeshIndexIdx = 0;
	bool bMemoryAllowFailed = false;
	while (nMeshIndexIdx < unIndexNum)
	{
		MMeshClusterData clusterData;
		if (!m_indexMemoryPool.AllowMemory(ClusterSize * pMesh->GetIndexStructSize(), clusterData.memoryInfo))
		{
			bMemoryAllowFailed = true;
			break;
		}

		std::array<uint32_t, ClusterSize> vClusterIndexData;

		size_t nClusterIndexIdx = 0;
		while (nClusterIndexIdx < ClusterSize)
		{
			uint32_t originIndex = vIndexData[(std::min)(nMeshIndexIdx + nClusterIndexIdx, unIndexNum - 1)];
			uint32_t globalIndex = unVertexBeginIndex + originIndex;
			vClusterIndexData[nClusterIndexIdx] = globalIndex;

			clusterData.boundsShpere.AddPoint(vVertex[originIndex].position);

			++nClusterIndexIdx;
		}

		indexClusterData.push_back(clusterData);
		pDevice->UploadBuffer(&m_indexBuffer, clusterData.memoryInfo.begin, reinterpret_cast<const MByte*>(vClusterIndexData.data()), clusterData.memoryInfo.size);

		
		nMeshIndexIdx += ClusterSize;
	}

	if (bMemoryAllowFailed)
	{
		m_vertexMemoryPool.FreeMemory(vertexMemoryInfo);

		for (MMeshClusterData& clusterData : indexClusterData)
		{
			m_indexMemoryPool.FreeMemory(clusterData.memoryInfo);
		}

		return false;
	}

	MMeshMergeData& meshMergeData = m_tMeshTable[pMesh];
	meshMergeData.vertexMemoryInfo = vertexMemoryInfo;
	meshMergeData.vIndexData = std::move(indexClusterData);

	return true;
}

void MMergeInstancingMesh::UnregisterMesh(MIMesh* pMesh)
{
	if (!pMesh)
	{
		return;
	}

	auto&& findResult = m_tMeshTable.find(pMesh);
	if (findResult == m_tMeshTable.end())
	{
		return;
	}

	MMeshMergeData& meshMergeData = findResult->second;

	
	m_vertexMemoryPool.FreeMemory(meshMergeData.vertexMemoryInfo);

	for(MMeshClusterData& clusterData : meshMergeData.vIndexData)
	{
		m_indexMemoryPool.FreeMemory(clusterData.memoryInfo);
	}

	m_tMeshTable.erase(findResult);
}

const MMergeInstancingMesh::MMeshMergeData& MMergeInstancingMesh::FindMesh(MIMesh* pMesh)
{
	auto&& findResult = m_tMeshTable.find(pMesh);
	if (findResult == m_tMeshTable.end())
	{
		static MMeshMergeData InvalidData;
		return InvalidData;
	}

	return findResult->second;
}
