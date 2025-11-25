#include "MMeshManager.h"

#include "Utility/MGlobal.h"
#include "Utility/MRenderGlobal.h"
#include "Engine/MEngine.h"
#include "Mesh/MCluster.h"
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
#include <algorithm>
#include <utility>
#include <vcruntime.h>

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMeshManager, MObject)

constexpr size_t VertexMemoryMaxSize = 1024 * 1024 * 20;
constexpr size_t IndexMemoryMaxSize  = 1024 * 1024 * 80;
constexpr size_t ClusterSize         = 64u * 3u;

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


size_t MMeshManager::RegisterClusterGroup(const MClusterGroup& group)
{
    MORTY_UNUSED(group);

    auto idx = m_clusterGroupDataIDPool.GetNewID();
    if (m_clusterGroupDatas.size() <= idx) { m_clusterGroupDatas.resize(idx + 1); }
    MClusterGroupData& data = m_clusterGroupDatas[idx];
    data.valid              = true;

    return idx;
}

void MMeshManager::UnregisterClusterGroup(const size_t& groupIdx)
{
    MORTY_ASSERT(groupIdx < m_clusterGroupDatas.size());
    m_clusterGroupDataIDPool.RecoveryID(groupIdx);
}

void MMeshManager::LoadClusterPage(size_t groupIdx, const MClusterPage& page)
{
    auto& clusterGroupData = m_clusterGroupDatas[groupIdx];
    m_vertexMemoryPool.AllocMemory(page.vertexData.size(), clusterGroupData.vertexMemoryInfo);
    m_indexMemoryPool.AllocMemory(page.indexData.size() * sizeof(MIndicesType), clusterGroupData.indexMemoryInfo);
    clusterGroupData.valid = true;

    {
        std::lock_guard lock(m_uploadMutex);
        m_uploadPageQueue.push_back(std::make_pair(groupIdx, page));
    }
}

void MMeshManager::UnloadClusterPage(const size_t& groupIdx)
{
    {
        std::lock_guard lock(m_uploadMutex);
        m_uploadPageQueue.erase(
                std::remove_if(
                        m_uploadPageQueue.begin(),
                        m_uploadPageQueue.end(),
                        [groupIdx](const auto& pair) { return pair.first == groupIdx; }
                ),
                m_uploadPageQueue.end()
        );
    }

    const auto& clusterGroupData = m_clusterGroupDatas[groupIdx];
    if (clusterGroupData.valid)
    {
        m_vertexMemoryPool.FreeMemory(clusterGroupData.vertexMemoryInfo);
        m_indexMemoryPool.FreeMemory(clusterGroupData.indexMemoryInfo);

        m_clusterGroupDatas[groupIdx].valid = false;
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

void MMeshManager::UploadPageData(size_t groupIdx, const MClusterPage& page)
{
    const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    MIDevice*            pDevice       = pRenderSystem->GetDevice();
    const auto&          groupData     = m_clusterGroupDatas[groupIdx];

    pDevice->UploadBuffer(
            &m_vertexBuffer,
            groupData.vertexMemoryInfo.begin,
            page.vertexData.data(),
            page.vertexData.size()
    );

    pDevice->UploadBuffer(
            &m_indexBuffer,
            groupData.indexMemoryInfo.begin,
            reinterpret_cast<const MByte*>(page.indexData.data()),
            page.indexData.size() * sizeof(MIndicesType)
    );
}

void MMeshManager::UploadBufferTask(MTaskNode* pNode)
{
    MORTY_UNUSED(pNode);

    if (m_uploadPageQueue.empty()) { return; }

    std::vector<std::pair<size_t, MClusterPage>> uploadQueue;

    {
        std::lock_guard lock(m_uploadMutex);
        uploadQueue.swap(m_uploadPageQueue);
    }

    for (const auto& [groupIdx, page]: uploadQueue) { UploadPageData(groupIdx, page); }
}

bool MMeshManager::RegisterMesh(MIMesh* mesh)
{
    if (!mesh) { return false; }

    if (m_meshTable.find(mesh) != m_meshTable.end()) { return true; }

    auto id = m_meshTable[mesh] = m_meshDataIDPool.GetNewID();

    if (m_meshDatas.size() <= id) { m_meshDatas.resize(id + 1); }

    auto& meshData = m_meshDatas[id] = MMeshData();

    meshData.clusterGroupIDs.resize(mesh->GetClusterGroup().size());
    std::transform(
            mesh->GetClusterGroup().begin(),
            mesh->GetClusterGroup().end(),
            meshData.clusterGroupIDs.begin(),
            [this](const MClusterGroup& group) { return RegisterClusterGroup(group); }
    );

    const auto& pages = mesh->GetClusterPages();
    for (size_t i = 0; i < pages.size(); ++i) { LoadClusterPage(meshData.clusterGroupIDs[i], pages[i]); }

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

    for (auto groupIdx: m_meshDatas[findResult->second].clusterGroupIDs)
    {
        UnregisterClusterGroup(groupIdx);
        UnloadClusterPage(groupIdx);
    }

    m_meshTable.erase(findResult);
}

bool    MMeshManager::HasMesh(MIMesh* pMesh) const { return m_meshTable.find(pMesh) != m_meshTable.end(); }

MIMesh* MMeshManager::GetScreenRect() const { return m_screenRect.get(); }

std::shared_ptr<MMeshBufferAdapter> MMeshManager::GetMeshBuffer() const { return m_meshBufferAdapter; }
