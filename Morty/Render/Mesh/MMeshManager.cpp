#include "MMeshManager.h"

#include "Utility/MGlobal.h"
#include "Engine/MEngine.h"
#include "Mesh/MMesh.h"
#include "Mesh/MMeshUtil.h"
#include "Mesh/MVertex.h"
#include "RHI/Abstract/MIDevice.h"
#include "Render/RenderGraph/MRenderCommon.h"
#include "Resource/MMeshResource.h"
#include "Resource/MMeshResourceUtil.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMeshManager, MObject)

constexpr size_t VertexMemoryMaxSize = 1024 * 1024 * 20;
constexpr size_t IndexMemoryMaxSize  = 1024 * 1024 * 80;
constexpr size_t ClusterSize         = 64u * 3u;

constexpr size_t VertexAllocByteAlignment = 32;

class MeshManagerBuffer : public MMeshBufferAdapter
{
public:
    explicit MeshManagerBuffer(const MMeshManager* pMeshManager)
        : pOwner(pMeshManager)
    {
        MORTY_ASSERT(pOwner);
    }

    [[nodiscard]] const MBuffer* GetVertexBuffer() const override { return pOwner->GetVertexBuffer(); }

    [[nodiscard]] const MBuffer* GetIndexBuffer() const override { return pOwner->GetIndexBuffer(); }

private:
    const MMeshManager* pOwner = nullptr;
};

MMeshManager::MMeshManager()
    : MeshVertexStructSize(sizeof(MVertex))
    , m_vertexMemoryPool(VertexMemoryMaxSize * MeshVertexStructSize)
    , m_indexMemoryPool(IndexMemoryMaxSize * sizeof(uint32_t))
{
    m_meshBufferAdapter = std::make_shared<MeshManagerBuffer>(this);
}

void MMeshManager::OnCreated()
{
    Super::OnCreated();

    const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    m_vertexBuffer = MBuffer::CreateVertexBuffer("MeshManager VertexBuffer");
    m_vertexBuffer.ReallocMemory(m_vertexMemoryPool.GetMaxMemorySize());
    m_vertexBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, 0);

    m_indexBuffer = MBuffer::CreateIndexBuffer("MeshManager VertexBuffer");
    m_indexBuffer.ReallocMemory(m_indexMemoryPool.GetMaxMemorySize());
    m_indexBuffer.GenerateBuffer(pRenderSystem->GetDevice(), nullptr, 0);

    InitializeScreenRect();

    auto* pUploadBufferTask = GetEngine()->GetMainGraph()->AddNode<MTaskNode>(MRenderGlobal::TASK_UPLOAD_MESH_UPDATE);
    pUploadBufferTask->SetThreadType(METhreadType::ERenderThread);
    pUploadBufferTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MMeshManager::UploadBufferTask, this));
}

void MMeshManager::OnDelete()
{
    ReleaseScreenRect();

    const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    m_vertexBuffer.DestroyBuffer(pRenderSystem->GetDevice());
    m_indexBuffer.DestroyBuffer(pRenderSystem->GetDevice());


    Super::OnDelete();
}

size_t MMeshManager::RegisterClusterMesh(const MClusterPage& page)
{
    auto id = m_clusterIDPool.GetNewID();
    if (id >= m_clusters.size()) { m_clusters.resize(id + 1); }

    MClusterData data;
    data.vertexMemoryInfo = vertexMemoryInfo;
    data.indexMemoryInfo  = indexMemoryInfo;
    data.bounds           = cluster.bounds;

    m_clusters[id] = data;

    {
        std::lock_guard lock(m_uploadMutex);
        m_uploadQueue.push_back(std::make_pair(cluster, id));
    }

    return id;
}

void MMeshManager::UnregisterClusterMesh(const size_t& clusterIdx)
{
    const auto& clusterData = m_clusters[clusterIdx];
    m_vertexMemoryPool.FreeMemory(clusterData.vertexMemoryInfo);
    m_indexMemoryPool.FreeMemory(clusterData.indexMemoryInfo);

    MORTY_ASSERT(clusterIdx < m_clusters.size());
    m_clusterIDPool.RecoveryID(clusterIdx);

    {
        std::lock_guard lock(m_uploadMutex);
        m_uploadQueue.erase(
                std::remove_if(
                        m_uploadQueue.begin(),
                        m_uploadQueue.end(),
                        [clusterIdx](const auto& pair) { return pair.second == clusterIdx; }
                ),
                m_uploadQueue.end()
        );
    }
}

void MMeshManager::InitializeScreenRect()
{
    m_screenRect = std::make_unique<MMesh<Vector2>>(true);
    m_screenRect->ResizeVertices(4);
    Vector2* vVertices = (Vector2*) m_screenRect->GetVertices();

    vVertices[0] = Vector2(-1, -1);
    vVertices[1] = Vector2(1, -1);
    vVertices[2] = Vector2(-1, 1);
    vVertices[3] = Vector2(1, 1);

    m_screenRect->ResizeIndices(2, 3);
    uint32_t* vIndices = m_screenRect->GetIndices();

    vIndices[0] = 0;
    vIndices[1] = 2;
    vIndices[2] = 1;

    vIndices[3] = 2;
    vIndices[4] = 3;
    vIndices[5] = 1;


    MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
    m_screenRect->GenerateBuffer(pRenderSystem->GetDevice());
}

void MMeshManager::ReleaseScreenRect()
{
    MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
    m_screenRect->DestroyBuffer(pRenderSystem->GetDevice());
    m_screenRect = nullptr;
}

size_t MMeshManager::RoundIndexSize(size_t unIndexNum)
{
    return size_t((unIndexNum / ClusterSize) + (unIndexNum % ClusterSize ? 1 : 0)) * ClusterSize;
    ;
}

void MMeshManager::UploadBuffer(MIMesh* pMesh)
{
    const MRenderSystem*  pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    MIDevice*             pDevice       = pRenderSystem->GetDevice();
    MMeshData&            meshMergeData = m_meshTable[pMesh];

    const MByte*          vVertexData        = pMesh->GetVertices();
    const MVertex*        vVertex            = reinterpret_cast<const MVertex*>(vVertexData);
    const uint32_t*       vIndexData         = pMesh->GetIndices();
    const size_t          unVertexSize       = pMesh->GetVerticesSize();
    const size_t          unIndexNum         = pMesh->GetIndicesNum();
    const size_t          unVertexStructSize = pMesh->GetVertexStructSize();
    const size_t          unRoundIndexNum    = RoundIndexSize(unIndexNum);
    const MemoryInfo&     vertexMemoryInfo   = meshMergeData.vertexMemoryInfo;
    const MemoryInfo&     indexMemoryInfo    = meshMergeData.indexMemoryInfo;

    //redirect index to global vertex space.
    std::vector<uint32_t> vRedirectIndex(unRoundIndexNum);

    MemoryInfo            indexInfo;
    indexInfo.begin = indexMemoryInfo.begin / sizeof(uint32_t);
    indexInfo.size  = indexMemoryInfo.size / sizeof(uint32_t);

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
    const size_t              nNewVertexIndexBegin = vertexInfo.begin / unVertexStructSize;


    size_t                    nCurrentIndex = 0;
    std::vector<MClusterData> meshClusterData;
    while (nCurrentIndex < unRoundIndexNum)
    {
        MClusterData         indexMemoryData;

        //get bounding sphere.
        std::vector<Vector3> boundsVertex(ClusterSize);

        for (uint32_t nIndexInCluster = 0; nIndexInCluster < ClusterSize; ++nIndexInCluster)
        {
            const uint32_t originIndex = vIndexData[(std::min) (nCurrentIndex + nIndexInCluster, unIndexNum - 1)];
            const uint32_t globalIndex = static_cast<uint32_t>(nNewVertexIndexBegin + originIndex);
            vRedirectIndex[nCurrentIndex + nIndexInCluster] = globalIndex;
            boundsVertex[nIndexInCluster]                   = vVertex[originIndex].position;
        }

        indexMemoryData.indexInfo.begin = indexInfo.begin + nCurrentIndex;
        indexMemoryData.indexInfo.size  = ClusterSize;

        indexMemoryData.boundsShpere
                .SetPoints(reinterpret_cast<const MByte*>(boundsVertex.data()), ClusterSize, 0, sizeof(Vector3));

        meshClusterData.push_back(indexMemoryData);

        nCurrentIndex += ClusterSize;
    }

    pDevice->UploadBuffer(&m_vertexBuffer, vertexInfo.begin, vVertexData, unVertexSize);

    pDevice->UploadBuffer(
            &m_indexBuffer,
            indexMemoryInfo.begin,
            reinterpret_cast<const MByte*>(vRedirectIndex.data()),
            indexMemoryInfo.size
    );

    meshMergeData.vertexInfo   = vertexInfo;
    meshMergeData.indexInfo    = indexInfo;
    meshMergeData.vClusterData = std::move(meshClusterData);
}

void MMeshManager::UploadBufferTask(MTaskNode* pNode)
{
    MORTY_UNUSED(pNode);

    if (m_uploadQueue.empty()) { return; }

    std::vector<MIMesh*> vUploadQueue;
    {
        std::lock_guard lock(m_uploadMutex);
        vUploadQueue.swap(m_uploadQueue);
    }

    for (MIMesh* pMesh: vUploadQueue) { UploadBuffer(pMesh); }
}

bool MMeshManager::RegisterMesh(MIMesh* mesh)
{
    if (!mesh) { return false; }

    if (m_meshTable.find(mesh) != m_meshTable.end()) { return true; }

    auto id = m_meshTable[mesh] = m_meshDataIDPool.GetNewID();

    if (m_meshDatas.size() <= id) { m_meshDatas.resize(id + 1); }

    auto& meshData = m_meshDatas[id] = MMeshData();

    meshData.clusters = mesh->GetClusters();
    meshData.groups   = mesh->GetClusterGroup();
    meshData.lods     = mesh->GetClusterLodData();

    return true;
}

void MMeshManager::UnregisterMesh(MIMesh* mesh)
{
    if (!mesh)
    {
        MORTY_ASSERT(mesh);
        return;
    }

    auto findResult = m_meshTable.find(mesh);
    if (findResult == m_meshTable.end())
    {
        MORTY_ASSERT(findResult != m_meshTable.end());
        return;
    }

    m_meshTable.erase(findResult);
}

bool    MMeshManager::HasMesh(MIMesh* pMesh) const { return m_meshTable.find(pMesh) != m_meshTable.end(); }

MIMesh* MMeshManager::GetScreenRect() const { return m_screenRect.get(); }

std::shared_ptr<MMeshBufferAdapter> MMeshManager::GetMeshBuffer() const { return m_meshBufferAdapter; }
