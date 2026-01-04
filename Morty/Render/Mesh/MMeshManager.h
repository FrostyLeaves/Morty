#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "Mesh/MCluster.h"
#include "Mesh/MMesh.h"
#include "Object/MObject.h"
#include "Scene/MManager.h"
#include "Utility/MBounds.h"
#include "Utility/MIDPool.h"
#include "Utility/MMemoryPool.h"

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
        MemoryInfo clusterInfo;
        MemoryInfo clusterGroupInfo;
        size_t     referenceNum = 0;
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
    [[nodiscard]] std::shared_ptr<MMeshBufferAdapter> GetMeshBuffer() const;

    MMeshRenderData                                   GetClusterRenderData(MIMesh* mesh) const;

private:
    void                                         LoadClusterPage(size_t groupIdx, const MClusterPage& page);
    void                                         UnloadClusterPage(const size_t& groupIdx);

    void                                         InitializeScreenRect();
    void                                         ReleaseScreenRect();

    size_t                                       RoundIndexSize(size_t nIndexSize);

    void                                         UploadPageData(size_t groupIdx, const MClusterPage& page);
    void                                         UploadClusterData();

    void                                         RenderUpdate(MTaskNode* node);

    const size_t                                 MeshVertexStructSize;

    MBuffer                                      m_vertexBuffer;
    MMemoryPool                                  m_vertexMemoryPool;

    MBuffer                                      m_indexBuffer;
    MMemoryPool                                  m_indexMemoryPool;

    std::unordered_map<MIMesh*, size_t>          m_meshTable;
    std::vector<MMeshData>                       m_meshDatas;
    MReusableIDPool<size_t>                      m_meshDataIDPool;

    std::unique_ptr<MIMesh>                      m_screenRect = nullptr;

    std::vector<MClusterGroupData>               m_clusterGroupDatas;
    MMemoryPool                                  m_clusterGroupDataIDPool;

    std::vector<MCluster>                        m_clusters;
    MMemoryPool                                  m_clusterPool;

    // Mesh resource data buffer
    std::vector<MMeshResourceData>               m_meshResourceDatas;
    MBuffer                                      m_meshResourceBuffer;

    // GPU buffers for cluster culling
    MBuffer                                      m_clusterGroupBuffer;
    MBuffer                                      m_clusterBuffer;

    // Dirty flag for cluster data upload
    bool                                         m_clustersDirty = false;

    // render thread.
    std::mutex                                   m_uploadMutex;
    std::vector<std::pair<size_t, MClusterPage>> m_uploadPageQueue;
    std::shared_ptr<MMeshBufferAdapter>          m_meshBufferAdapter = nullptr;

};

}// namespace morty