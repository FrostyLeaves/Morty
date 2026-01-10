#include "MMeshManager.h"

#include "Utility/MGlobal.h"
#include "Utility/MRenderGlobal.h"
#include "Component/MRenderMeshComponent.h"
#include "Engine/MEngine.h"
#include "MRenderNotify.h"
#include "Mesh/MCluster.h"
#include "Mesh/MMesh.h"
#include "Mesh/MMeshUtil.h"
#include "Mesh/MVertex.h"
#include "RHI/Abstract/MIDevice.h"
#include "Render/RenderGraph/MRenderCommon.h"
#include "Resource/MMeshResource.h"
#include "Resource/MMeshResourceUtil.h"
#include "Scene/MScene.h"
#include "System/MNotifyManager.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"


using namespace morty;

MORTY_CLASS_IMPLEMENT(MMeshManager, IManager)

constexpr size_t VertexMemoryMaxSize = 1024 * 1024 * 20;
constexpr size_t IndexMemoryMaxSize  = 1024 * 1024 * 80;
constexpr size_t ClusterGroupMaxSize = 1024 * 256;
constexpr size_t ClusterMaxSize      = 1024 * 1024 * 4;
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
    , m_clusterGroupDataIDPool(ClusterGroupMaxSize)
    , m_clusterPool(ClusterMaxSize)
{
    m_meshBufferAdapter = std::make_shared<MeshManagerBuffer>(this);
}

void MMeshManager::Initialize()
{
    Super::Initialize();

    const MRenderSystem* renderSystem = GetEngine()->GetSystem<MRenderSystem>();

    m_vertexBuffer = MBuffer::CreateVertexBuffer("MeshManager VertexBuffer");
    m_vertexBuffer.ReallocMemory(m_vertexMemoryPool.GetMaxMemorySize());
    m_vertexBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_vertexBuffer.GenerateBuffer(renderSystem->GetDevice(), nullptr, 0);

    m_indexBuffer = MBuffer::CreateIndexBuffer("MeshManager VertexBuffer");
    m_indexBuffer.ReallocMemory(m_indexMemoryPool.GetMaxMemorySize());
    m_indexBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_indexBuffer.GenerateBuffer(renderSystem->GetDevice(), nullptr, 0);

    m_clusterGroupBuffer = MBuffer::CreateStorageBuffer("MeshManager ClusterGroupBuffer");
    m_clusterBuffer      = MBuffer::CreateStorageBuffer("MeshManager ClusterBuffer");
    m_meshResourceBuffer = MBuffer::CreateStorageBuffer("MeshManager MeshResourceBuffer");

    InitializeScreenRect();

    m_uploadBufferTask = GetEngine()->GetMainGraph()->AddNode<MTaskNode>(MRenderGlobal::TASK_UPLOAD_MESH_UPDATE);
    m_uploadBufferTask->SetThreadType(METhreadType::ERenderThread);
    m_uploadBufferTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MMeshManager::RenderUpdate, this));
}

void MMeshManager::Release()
{
    if (m_uploadBufferTask)
    {
        GetEngine()->GetMainGraph()->DestroyNode(m_uploadBufferTask);
        m_uploadBufferTask = nullptr;
    }

    ReleaseScreenRect();

    const MRenderSystem* renderSystem = GetEngine()->GetSystem<MRenderSystem>();

    m_vertexBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_indexBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_clusterGroupBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_clusterBuffer.DestroyBuffer(renderSystem->GetDevice());
    m_meshResourceBuffer.DestroyBuffer(renderSystem->GetDevice());


    Super::Release();
}

void MMeshManager::LoadClusterPage(size_t groupIdx, const MClusterPage& page)
{
    auto& clusterGroupData = m_clusterGroupDatas[groupIdx];
    m_vertexMemoryPool.AllocMemory(page.vertexData.size(), clusterGroupData.vertexMemoryInfo);
    m_indexMemoryPool.AllocMemory(page.indexData.size() * sizeof(MIndicesType), clusterGroupData.indexMemoryInfo);
    clusterGroupData.renderData.valid = 1;

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
    if (clusterGroupData.renderData.valid)
    {
        m_vertexMemoryPool.FreeMemory(clusterGroupData.vertexMemoryInfo);
        m_indexMemoryPool.FreeMemory(clusterGroupData.indexMemoryInfo);

        m_clusterGroupDatas[groupIdx].renderData.valid = false;
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


    MRenderSystem* renderSystem = GetEngine()->GetSystem<MRenderSystem>();
    m_screenRect->DestroyBuffer(renderSystem->GetDevice());
    m_screenRect->GenerateBuffer(renderSystem->GetDevice());
}

void MMeshManager::ReleaseScreenRect()
{
    MRenderSystem* renderSystem = GetEngine()->GetSystem<MRenderSystem>();
    m_screenRect->DestroyBuffer(renderSystem->GetDevice());
    m_screenRect = nullptr;
}

size_t MMeshManager::RoundIndexSize(size_t unIndexNum)
{
    return size_t((unIndexNum / ClusterSize) + (unIndexNum % ClusterSize ? 1 : 0)) * ClusterSize;
    ;
}

void MMeshManager::UploadPageData(size_t groupIdx, const MClusterPage& page)
{
    const MRenderSystem* renderSystem = GetEngine()->GetSystem<MRenderSystem>();
    MIDevice*            pDevice      = renderSystem->GetDevice();
    const auto&          groupData    = m_clusterGroupDatas[groupIdx];

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

void MMeshManager::UploadClusterData()
{
    const MRenderSystem* renderSystem = GetEngine()->GetSystem<MRenderSystem>();
    MIDevice*            pDevice      = renderSystem->GetDevice();

    // Upload ClusterGroupData - directly use MClusterGroupRenderData (matches Shader layout)
    if (!m_clusterGroupDatas.empty())
    {
        std::vector<MClusterGroupRenderData> clusterGroupShaderData;
        clusterGroupShaderData.reserve(m_clusterGroupDatas.size());

        for (const auto& groupData: m_clusterGroupDatas) { clusterGroupShaderData.push_back(groupData.renderData); }

        if (!clusterGroupShaderData.empty())
        {
            m_clusterGroupBuffer.ApplyData(pDevice, clusterGroupShaderData);
        }
    }

    // Upload ClusterData - directly use MClusterRenderData (matches Shader layout)
    if (!m_clusters.empty())
    {
        std::vector<MClusterRenderData> clusterShaderData;
        clusterShaderData.reserve(m_clusters.size());

        for (const auto& cluster: m_clusters)
        {
            MClusterRenderData shaderData{};
            shaderData.indexOffset = cluster.indicesOffset;
            shaderData.indexCount  = cluster.indicesNum;
            shaderData.valid       = 1;
            shaderData.position    = cluster.bounds.position;
            shaderData.radius      = cluster.bounds.radius;
            shaderData.error       = cluster.bounds.error;
            clusterShaderData.push_back(shaderData);
        }

        m_clusterBuffer.ApplyData(pDevice, clusterShaderData);
    }

    // Upload MeshResourceData
    if (!m_meshResourceDatas.empty())
    {
        m_meshResourceBuffer.ApplyData(pDevice, m_meshResourceDatas);
    }

    m_clustersDirty = false;
}

void MMeshManager::RenderUpdate(MTaskNode* node)
{
    MORTY_UNUSED(node);

    // Upload page data
    if (!m_uploadPageQueue.empty())
    {
        std::vector<std::pair<size_t, MClusterPage>> uploadQueue;

        {
            std::lock_guard lock(m_uploadMutex);
            uploadQueue.swap(m_uploadPageQueue);
        }

        for (const auto& [groupIdx, page]: uploadQueue) { UploadPageData(groupIdx, page); }
    }

    // Upload cluster data to GPU
    UploadClusterData();
}

bool MMeshManager::RegisterMesh(MIMesh* mesh)
{
    if (!mesh) { return false; }

    auto findResult = m_meshTable.find(mesh);
    if (findResult != m_meshTable.end())
    {
        auto* meshData = m_meshDataPool.Get(findResult->second);
        MORTY_ASSERT(meshData);
        if (!meshData) { return false; }
        meshData->referenceNum++;
        return true;
    }

    auto id = m_meshDataPool.Emplace();
    m_meshTable[mesh] = id;

    // Populate MeshResourceData for this mesh
    if (m_meshResourceDatas.size() <= id) { m_meshResourceDatas.resize(id + 1); }

    const auto& clusters = mesh->GetClusters();

    auto*       meshData = m_meshDataPool.Get(id);
    MORTY_ASSERT(meshData);
    if (!meshData) { return false; }
    *meshData = MMeshData();

    m_clusterPool.AllocMemory(clusters.size(), meshData->clusterInfo);
    meshData->referenceNum = 1;

    //fill cluster data
    if (m_clusters.size() < meshData->clusterInfo.begin + clusters.size())
    {
        m_clusters.resize(meshData->clusterInfo.begin + clusters.size());
    }
    std::copy(clusters.begin(), clusters.end(), m_clusters.begin() + meshData->clusterInfo.begin);

    // Allocate cluster group IDs
    m_clusterGroupDataIDPool.AllocMemory(mesh->GetClusterGroup().size(), meshData->clusterGroupInfo);

    // Fill MeshResourceData - store root cluster group info
    m_meshResourceDatas[id].rootClusterGroupBeginIndex = static_cast<int32_t>(meshData->clusterGroupInfo.begin);
    // Root groups are at LOD level 0
    const auto& lods = mesh->GetClusterLodData();
    if (!lods.empty()) { m_meshResourceDatas[id].rootClusterGroupCount = static_cast<int32_t>(lods[0].groupNum); }
    else
    {
        // No LOD data, treat all groups as root groups
        m_meshResourceDatas[id].rootClusterGroupCount = static_cast<int32_t>(mesh->GetClusterGroup().size());
    }

    if (m_clusterGroupDatas.size() < meshData->clusterGroupInfo.begin + meshData->clusterGroupInfo.size)
    {
        m_clusterGroupDatas.resize(meshData->clusterGroupInfo.begin + meshData->clusterGroupInfo.size);
    }

    //fill cluster group render data
    for (size_t i = 0; i < mesh->GetClusterGroup().size(); ++i)
    {
        MClusterGroupData& data   = m_clusterGroupDatas[meshData->clusterGroupInfo.begin + i];
        auto&              source = mesh->GetClusterGroup()[i];

        data.renderData.childGroupCount   = source.childGroupCount;
        data.renderData.clusterBeginIndex = meshData->clusterInfo.begin + source.clusterOffset;
        data.renderData.clusterCount      = source.clusterNum;
        data.renderData.error             = source.bounds.error;
        data.renderData.firstGroupId      = meshData->clusterGroupInfo.begin + source.firstChildGroupId;
        data.renderData.parentGroupId     = meshData->clusterGroupInfo.begin + source.parentGroupId;
        data.renderData.position          = source.bounds.position;
        data.renderData.radius            = source.bounds.radius;
        data.renderData.valid             = 0;
    }

    //load vertex and index data
    const auto& pages = mesh->GetClusterPages();
    for (size_t i = 0; i < pages.size(); ++i) { LoadClusterPage(meshData->clusterGroupInfo.begin + i, pages[i]); }

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

    auto* meshData = m_meshDataPool.Get(findResult->second);
    MORTY_ASSERT(meshData);
    if (!meshData) { return; }
    meshData->referenceNum--;

    if (meshData->referenceNum != 0) { return; }

    for (auto groupIdx = meshData->clusterGroupInfo.begin;
         groupIdx < meshData->clusterGroupInfo.begin + meshData->clusterGroupInfo.size;
         ++groupIdx)
    {
        MORTY_ASSERT(groupIdx < m_clusterGroupDatas.size());
        m_clusterGroupDatas[groupIdx].renderData.valid = 0;
        UnloadClusterPage(groupIdx);
    }

    const auto meshId = findResult->second;
    m_meshResourceDatas[meshId] = MMeshResourceRenderData();
    m_meshTable.erase(findResult);
    m_meshDataPool.Release(meshId);
}

bool    MMeshManager::HasMesh(MIMesh* mesh) const { return m_meshTable.find(mesh) != m_meshTable.end(); }

MIMesh* MMeshManager::GetScreenRect() const { return m_screenRect.get(); }

std::shared_ptr<MMeshBufferAdapter> MMeshManager::GetMeshBuffer() const { return m_meshBufferAdapter; }

int32_t                             MMeshManager::GetMeshResourceId(MIMesh* mesh) const
{
    auto findResult = m_meshTable.find(mesh);
    if (findResult == m_meshTable.end()) { return MGlobal::M_INVALID_INT; }
    return static_cast<int32_t>(findResult->second);
}

size_t MMeshManager::GetRenderIndex(MIMesh* mesh) const
{
    auto findResult = m_meshTable.find(mesh);
    if (findResult == m_meshTable.end()) { return MGlobal::M_INVALID_UINDEX; }
    return findResult->second;
}
