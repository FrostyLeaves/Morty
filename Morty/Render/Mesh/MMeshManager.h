#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "Mesh/MMesh.h"
#include "Object/MObject.h"
#include "Utility/MBounds.h"
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
    void     OnCreated() override;
    void     OnDelete() override;

    struct MClusterData {
        MemoryInfo    indexInfo;
        MBoundsSphere boundsShpere;
    };

    struct MMeshData {
        MemoryInfo                vertexInfo;      //vertex offset and size.
        MemoryInfo                vertexMemoryInfo;//memory allow need byte alignment.
        MemoryInfo                indexInfo;//indexInfo.offset is indexMemoryInfo.offset divide by sizeof(uint32_t)
        MemoryInfo                indexMemoryInfo;
        std::vector<MClusterData> vClusterData;
    };

public:
    bool                           RegisterMesh(MIMesh* pMesh);

    void                           UnregisterMesh(MIMesh* pMesh);

    bool                           HasMesh(MIMesh* pMesh) const;

    const MMeshData&               FindMesh(MIMesh* pMesh) const;

    [[nodiscard]] MIMesh*          GetScreenRect() const;

public:
    [[nodiscard]] const MBuffer*                      GetVertexBuffer() const { return &m_vertexBuffer; }

    [[nodiscard]] const MBuffer*                      GetIndexBuffer() const { return &m_indexBuffer; }

    [[nodiscard]] std::shared_ptr<MMeshBufferAdapter> GetMeshBuffer() const;

private:
    void                                InitializeScreenRect();

    void                                ReleaseScreenRect();

    void                                InitializeSkyBox();

    void                                ReleaseSkyBox();

    void                                InitializeCube();

    void                                ReleaseCube();

    size_t                              RoundIndexSize(size_t nIndexSize);

    void                                UploadBuffer(MIMesh* pMesh);

    void                                UploadBufferTask(MTaskNode* pNode);

    const size_t                        MeshVertexStructSize;

    MBuffer                             m_vertexBuffer;
    MMemoryPool                         m_vertexMemoryPool;

    MBuffer                             m_indexBuffer;
    MMemoryPool                         m_indexMemoryPool;

    std::map<MIMesh*, MMeshData>        m_meshTable;

    std::unique_ptr<MIMesh>             m_screenRect = nullptr;


    // render thread.
    std::mutex                          m_uploadMutex;
    std::vector<MIMesh*>                m_uploadQueue;
    std::shared_ptr<MMeshBufferAdapter> m_meshBufferAdapter = nullptr;
};

}// namespace morty