#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "Mesh/MCluster.h"
#include "Mesh/MMesh.h"
#include "Object/MObject.h"
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
class MMeshBufferAdapter;
class MRenderMeshComponent;
class MMeshManager : public MObject
{
    MORTY_CLASS(MMeshManager)
public:
    explicit MMeshManager();
    void OnCreated() override;
    void OnDelete() override;

    struct MClusterData {

        MClusterBounds bounds;
    };

    struct MClusterRenderData {
        uint32_t indexOffset = 0;
        uint32_t indexCount  = 0;
        uint32_t parent      = MGlobal::M_INVALID_UINDEX;
        uint32_t firstChild  = MGlobal::M_INVALID_UINDEX;
        uint32_t childCount  = 0;
        uint32_t valid       = 0;

        Vector3  position;
        float    radius;
        float    error;
    };

    struct MMeshRenderData {
        uint32_t rootClusterOffset = MGlobal::M_INVALID_UINDEX;
        uint32_t rootClusterCount  = 0;
    };

    struct MClusterGroupData {
        bool       valid = false;
        MemoryInfo vertexMemoryInfo;
        MemoryInfo indexMemoryInfo;
    };

    struct MMeshData {
        std::vector<size_t> clusterGroupIDs;
    };

public:
    bool                                              RegisterMesh(MIMesh* mesh);
    void                                              UnregisterMesh(MIMesh* mesh);
    bool                                              HasMesh(MIMesh* mesh) const;
    size_t                                            GetRenderIndex(MIMesh* mesh) const;
    [[nodiscard]] MIMesh*                             GetScreenRect() const;
    [[nodiscard]] const MBuffer*                      GetVertexBuffer() const { return &m_vertexBuffer; }
    [[nodiscard]] const MBuffer*                      GetIndexBuffer() const { return &m_indexBuffer; }
    [[nodiscard]] std::shared_ptr<MMeshBufferAdapter> GetMeshBuffer() const;

private:
    size_t                                       RegisterClusterGroup(const MClusterGroup& group);
    void                                         UnregisterClusterGroup(const size_t& groupIdx);

    void                                         LoadClusterPage(size_t groupIdx, const MClusterPage& page);
    void                                         UnloadClusterPage(const size_t& groupIdx);

    void                                         InitializeScreenRect();
    void                                         ReleaseScreenRect();

    size_t                                       RoundIndexSize(size_t nIndexSize);

    void                                         UploadPageData(size_t groupIdx, const MClusterPage& page);

    void                                         UploadBufferTask(MTaskNode* pNode);

    const size_t                                 MeshVertexStructSize;

    MBuffer                                      m_vertexBuffer;
    MMemoryPool                                  m_vertexMemoryPool;

    MBuffer                                      m_indexBuffer;
    MMemoryPool                                  m_indexMemoryPool;

    std::unordered_map<MIMesh*, size_t>          m_meshTable;
    std::vector<MMeshData>                       m_meshDatas;
    MRepeatIDPool<size_t>                        m_meshDataIDPool;

    std::unique_ptr<MIMesh>                      m_screenRect = nullptr;


    std::vector<MClusterGroupData>               m_clusterGroupDatas;
    MRepeatIDPool<size_t>                        m_clusterGroupDataIDPool;


    // render thread.
    std::mutex                                   m_uploadMutex;
    std::vector<std::pair<size_t, MClusterPage>> m_uploadPageQueue;
    std::shared_ptr<MMeshBufferAdapter>          m_meshBufferAdapter = nullptr;
};

}// namespace morty