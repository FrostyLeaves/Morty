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

constexpr size_t VertexMemoryMaxSize     = 1024 * 1024 * 20;
constexpr size_t IndexMemoryMaxSize      = 1024 * 1024 * 80;
constexpr size_t ClusterGroupMaxSize     = 1024 * 256;
constexpr size_t ClusterMaxSize          = 1024 * 1024 * 4;
constexpr size_t ClusterSize             = 64u * 3u;
constexpr size_t MeshResourceDefaultSize = 1024;

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
    , m_clusterPool(ClusterMaxSize)
    , m_clusterGroupPool(ClusterGroupMaxSize)
    , m_meshResourcePool(MeshResourceDefaultSize)
    , m_groupLinkPool(ClusterGroupMaxSize * 32)
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

    m_clusterBuffer      = MBuffer::CreateStorageBuffer("MeshManager ClusterBuffer");
    m_clusterGroupBuffer = MBuffer::CreateStorageBuffer("MeshManager ClusterGroupBuffer");
    m_meshResourceBuffer = MBuffer::CreateStorageBuffer("MeshManager MeshResourceBuffer");
    m_groupLinksBuffer   = MBuffer::CreateStorageBuffer("MeshManager GroupLinksBuffer");

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

void MMeshManager::LoadClusterPage(MMeshData* meshData, size_t groupIdx, const MClusterPage& page)
{
    m_vertexMemoryPool.AllocMemory(page.vertexData.size(), meshData->pageMemoryInfo[groupIdx].vertexMemoryInfo);
    m_indexMemoryPool.AllocMemory(page.indexData.size() * sizeof(MIndicesType), meshData->pageMemoryInfo[groupIdx].indexMemoryInfo);
    meshData->pageMemoryInfo[groupIdx].loaded = true;

    PageDataUploadRequest request;
    request.globalGroupIdx = meshData->clusterGroupInfo.begin + groupIdx;
    request.pageData       = page;
    request.memoryData     = meshData->pageMemoryInfo[groupIdx];

    {
        std::lock_guard lock(m_uploadMutex);
        m_uploadPageQueue.emplace_back(request);
    }
}

void MMeshManager::UnloadClusterPage(MMeshData* meshData, size_t groupIdx)
{
    size_t globalGroupIdx = meshData->clusterGroupInfo.begin + groupIdx;

    {
        std::lock_guard lock(m_uploadMutex);
        m_uploadPageQueue.erase(
                std::remove_if(m_uploadPageQueue.begin(), m_uploadPageQueue.end(), [globalGroupIdx](const auto& pair) { return pair.globalGroupIdx == globalGroupIdx; }),
                m_uploadPageQueue.end()
        );
    }

    const auto& memoryData = meshData->pageMemoryInfo[groupIdx];
    if (!memoryData.loaded) { return; }

    m_vertexMemoryPool.FreeMemory(memoryData.vertexMemoryInfo);
    m_indexMemoryPool.FreeMemory(memoryData.indexMemoryInfo);
    meshData->pageMemoryInfo[groupIdx].loaded = false;
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

void MMeshManager::UploadPageData(const PageDataUploadRequest& request)
{
    const MRenderSystem* renderSystem = GetEngine()->GetSystem<MRenderSystem>();
    MIDevice*            pDevice      = renderSystem->GetDevice();

    pDevice->UploadBuffer(&m_vertexBuffer, request.memoryData.vertexMemoryInfo.begin, request.pageData.vertexData.data(), request.pageData.vertexData.size());
    pDevice->UploadBuffer(
            &m_indexBuffer,
            request.memoryData.indexMemoryInfo.begin,
            reinterpret_cast<const MByte*>(request.pageData.indexData.data()),
            request.pageData.indexData.size() * sizeof(MIndicesType)
    );
}

void MMeshManager::UploadClusterData(const ClusterDataUploadRequest& request)
{
    const MRenderSystem* renderSystem = GetEngine()->GetSystem<MRenderSystem>();
    MIDevice*            pDevice      = renderSystem->GetDevice();


    if (!request.clusters.empty())
    {
        m_clusterBuffer.ApplyData(
                pDevice,
                request.meshData.clusterInfo.begin * sizeof(MClusterRenderData),
                reinterpret_cast<const MByte*>(request.clusters.data()),
                request.clusters.size() * sizeof(MClusterRenderData)
        );
    }

    // Upload ClusterGroupData - directly use MClusterGroupRenderData (matches Shader layout)
    if (!request.clusterGroups.empty())
    {
        m_clusterGroupBuffer.ApplyData(
                pDevice,
                request.meshData.clusterGroupInfo.begin * sizeof(MClusterGroupRenderData),
                reinterpret_cast<const MByte*>(request.clusterGroups.data()),
                request.clusterGroups.size() * sizeof(MClusterGroupRenderData)
        );
    }

    // Upload MeshResourceData
    m_meshResourceBuffer.ApplyData(pDevice, request.meshData.meshResourceId * sizeof(MMeshResourceRenderData), reinterpret_cast<const MByte*>(&request.renderData), sizeof(MMeshResourceRenderData));

    // Upload Root Grups index.
    m_groupLinksBuffer
            .ApplyData(pDevice, request.meshData.rootGroupLinksInfo.begin * sizeof(uint32_t), reinterpret_cast<const MByte*>(request.rootGroups.data()), request.rootGroups.size() * sizeof(uint32_t));

    // Upload groups link.
    m_groupLinksBuffer
            .ApplyData(pDevice, request.meshData.groupLinksInfo.begin * sizeof(uint32_t), reinterpret_cast<const MByte*>(request.groupLinks.data()), request.groupLinks.size() * sizeof(uint32_t));
}

void MMeshManager::RenderUpdate(MTaskNode* node)
{
    MORTY_UNUSED(node);


    if (!m_uploadClusterQueue.empty())
    {
        std::vector<ClusterDataUploadRequest> uploadQueue;

        {
            std::lock_guard lock(m_uploadMutex);
            uploadQueue.swap(m_uploadClusterQueue);
        }

        for (const auto& request: uploadQueue) { UploadClusterData(request); }
    }

    // Upload page data
    if (!m_uploadPageQueue.empty())
    {
        std::vector<PageDataUploadRequest> uploadQueue;

        {
            std::lock_guard lock(m_uploadMutex);
            uploadQueue.swap(m_uploadPageQueue);
        }

        for (const auto& request: uploadQueue) { UploadPageData(request); }
    }
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

    auto id           = m_meshDataPool.Emplace();
    m_meshTable[mesh] = id;

    auto* meshData = m_meshDataPool.Get(id);
    MORTY_ASSERT(meshData);
    if (!meshData) { return false; }
    *meshData                = MMeshData();
    meshData->referenceNum   = 1;
    meshData->meshResourceId = id;

    ClusterDataUploadRequest request;

    {
        //fill cluster data
        request.clusters.resize(mesh->GetClusters().size());
        m_clusterPool.AllocMemory(request.clusters.size(), meshData->clusterInfo);

        for (size_t idx = 0; idx < request.clusters.size(); ++idx)
        {
            const auto& cluster = mesh->GetClusters()[idx];
            auto&       target  = request.clusters[idx];

            target.position    = cluster.bounds.position;
            target.radius      = cluster.bounds.radius;
            target.error       = cluster.bounds.error;
            target.indexOffset = cluster.indicesOffset;
            target.indexCount  = cluster.indicesNum;
        }
    }

    {
        request.rootGroups = mesh->GetRootGroups();
        m_groupLinkPool.AllocMemory(request.rootGroups.size(), meshData->rootGroupLinksInfo);

        request.renderData.clusterBeginIndex      = static_cast<int32_t>(meshData->clusterInfo.begin);
        request.renderData.clusterGroupBeginIndex = static_cast<int32_t>(meshData->clusterGroupInfo.begin);
        request.renderData.rootClusterGroupIndex  = static_cast<int32_t>(meshData->rootGroupLinksInfo.begin);
        request.renderData.rootClusterGroupCount  = static_cast<int32_t>(meshData->rootGroupLinksInfo.size);

        // Collect group links
        request.groupLinks = mesh->GetGroupLinks();
        m_groupLinkPool.AllocMemory(request.groupLinks.size(), meshData->groupLinksInfo);
    }

    {
        //load vertex and index data
        const auto& pages = mesh->GetClusterPages();
        meshData->pageMemoryInfo.resize(pages.size());

        for (size_t i = 0; i < pages.size(); ++i) { LoadClusterPage(meshData, i, pages[i]); }
    }

    {
        // Allocate cluster group IDs
        request.clusterGroups.resize(mesh->GetClusterGroup().size());
        //fill cluster group render data
        for (size_t i = 0; i < request.clusterGroups.size(); ++i)
        {
            MClusterGroupRenderData& data   = request.clusterGroups[i];
            auto&                    source = mesh->GetClusterGroup()[i];

            // groupLinkOffset is global offset into groupLinks buffer
            data.groupLinkOffset   = meshData->groupLinksInfo.begin + source.groupLinkOffset;
            data.groupLinkCount    = source.groupLinkCount;
            data.clusterBeginIndex = meshData->clusterInfo.begin + source.clusterOffset;
            data.clusterCount      = source.clusterNum;
            data.vertexBeginIndex  = meshData->pageMemoryInfo[i].vertexMemoryInfo.begin / mesh->GetVertexStructSize();
            data.indexBeginIndex   = meshData->pageMemoryInfo[i].indexMemoryInfo.begin / mesh->GetIndexStructSize();
            data.error             = source.bounds.error;
            data.position          = source.bounds.position;
            data.radius            = source.bounds.radius;
            data.valid             = 0;
        }

        m_clusterGroupPool.AllocMemory(request.clusterGroups.size(), meshData->clusterGroupInfo);
    }

    request.meshData = *meshData;
    {
        std::lock_guard lock(m_uploadMutex);
        m_uploadClusterQueue.emplace_back(request);
    }
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

    // Unload all cluster pages (releases vertex and index memory)
    for (size_t pageIdx = 0; pageIdx < meshData->pageMemoryInfo.size(); ++pageIdx) { UnloadClusterPage(meshData, pageIdx); }

    // Release cluster pool memory
    m_clusterPool.FreeMemory(meshData->clusterInfo);

    // Release cluster group pool memory
    m_clusterGroupPool.FreeMemory(meshData->clusterGroupInfo);

    // Release group links pool memory
    m_groupLinkPool.FreeMemory(meshData->groupLinksInfo);
    m_groupLinkPool.FreeMemory(meshData->rootGroupLinksInfo);

    const auto meshId = findResult->second;
    m_meshTable.erase(findResult);
    m_meshDataPool.Release(meshId);
}

bool                                MMeshManager::HasMesh(MIMesh* mesh) const { return m_meshTable.find(mesh) != m_meshTable.end(); }

MIMesh*                             MMeshManager::GetScreenRect() const { return m_screenRect.get(); }

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
