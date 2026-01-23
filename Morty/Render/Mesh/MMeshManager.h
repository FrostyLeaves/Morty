#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "Container/MItemPool.h"
#include "Mesh/MCluster.h"
#include "Mesh/MMesh.h"
#include "Object/MObject.h"
#include "Scene/MManager.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"
#include "Utility/MPoolAllocator.h"

namespace morty
{

class MTaskNode;
class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MMeshResource;
class MMeshBufferAdapter;
class MMeshManager : public IManager
{
    MORTY_CLASS(MMeshManager)

    struct MMeshData {
        MemoryInfo                          clusterInfo;
        MemoryInfo                          clusterGroupInfo;
        MemoryInfo                          groupLinksInfo;
        MemoryInfo                          rootGroupLinksInfo;
        std::vector<MClusterPageMemoryData> pageMemoryInfo;
        size_t                              meshResourceId = 0;
        size_t                              referenceNum   = 0;
    };

public:
    explicit                                          MMeshManager();
    void                                              Initialize() override;
    void                                              Release() override;

    bool                                              RegisterMesh(MIMesh* mesh);
    void                                              UnregisterMesh(MIMesh* mesh);
    bool                                              HasMesh(MIMesh* mesh) const;
    size_t                                            GetRenderIndex(MIMesh* mesh) const;
    [[nodiscard]] int32_t                             GetMeshResourceId(MIMesh* mesh) const;
    [[nodiscard]] MIMesh*                             GetScreenRect() const;
    [[nodiscard]] const MBuffer*                      GetVertexBuffer() const { return &m_vertexBuffer; }
    [[nodiscard]] const MBuffer*                      GetIndexBuffer() const { return &m_indexBuffer; }
    [[nodiscard]] const MBuffer*                      GetClusterGroupBuffer() const { return &m_clusterGroupBuffer; }
    [[nodiscard]] const MBuffer*                      GetClusterBuffer() const { return &m_clusterBuffer; }
    [[nodiscard]] const MBuffer*                      GetMeshResourceBuffer() const { return &m_meshResourceBuffer; }
    [[nodiscard]] const MBuffer*                      GetGroupLinksBuffer() const { return &m_groupLinksBuffer; }
    [[nodiscard]] std::shared_ptr<MMeshBufferAdapter> GetMeshBuffer() const;

private:
    struct PageDataUploadRequest {
        size_t                 globalGroupIdx = 0;
        MClusterPage           pageData;
        MClusterPageMemoryData memoryData;
    };

    struct ClusterDataUploadRequest {
        MMeshData                            meshData;

        std::vector<MClusterRenderData>      clusters;
        std::vector<MClusterGroupRenderData> clusterGroups;
        std::vector<int32_t>                 groupLinks;
        std::vector<uint32_t>                rootGroups;

        MMeshResourceRenderData              renderData;
    };


    void                                  LoadClusterPage(MMeshData* meshData, size_t groupIdx, const MClusterPage& page);
    void                                  UnloadClusterPage(MMeshData* meshData, size_t groupIdx);

    void                                  InitializeScreenRect();
    void                                  ReleaseScreenRect();

    size_t                                RoundIndexSize(size_t nIndexSize);

    void                                  UploadPageData(const PageDataUploadRequest& request);
    void                                  UploadClusterData(const ClusterDataUploadRequest& request);

    void                                  RenderUpdate(MTaskNode* node);

    const size_t                          MeshVertexStructSize;

    std::unique_ptr<MIMesh>               m_screenRect = nullptr;

    std::unordered_map<MIMesh*, size_t>   m_meshTable;
    MItemPool<MMeshData>                  m_meshDataPool;

    MBuffer                               m_vertexBuffer;
    MMemoryPool                           m_vertexMemoryPool;

    MBuffer                               m_indexBuffer;
    MMemoryPool                           m_indexMemoryPool;

    MBuffer                               m_meshResourceBuffer;
    MMemoryPool                           m_meshResourcePool;

    MBuffer                               m_clusterBuffer;
    MMemoryPool                           m_clusterPool;

    MBuffer                               m_clusterGroupBuffer;
    MMemoryPool                           m_clusterGroupPool;

    MBuffer                               m_groupLinksBuffer;
    MMemoryPool                           m_groupLinkPool;

    // render thread.

    std::mutex                            m_uploadMutex;
    std::vector<PageDataUploadRequest>    m_uploadPageQueue;
    std::vector<ClusterDataUploadRequest> m_uploadClusterQueue;
    std::shared_ptr<MMeshBufferAdapter>   m_meshBufferAdapter = nullptr;

    MTaskNode*                            m_uploadBufferTask = nullptr;
};

}// namespace morty
