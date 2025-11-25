/**
 * @File         MMesh
 * 
 * @Created      2019-08-06 17:29:47
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Basic/MBuffer.h"
#include "MCluster.h"

namespace morty
{

class MIDevice;
class MBuffer;
struct MVertex;

class MMaterial;

class MORTY_API MIMesh
{
public:
    explicit MIMesh(const bool& nDynamicMesh = false);

    virtual ~MIMesh();

    void                                           SetDirty();
    void                                           GenerateBuffer(MIDevice* pDevice);
    void                                           UploadBuffer(MIDevice* pDevice);
    void                                           DestroyBuffer(MIDevice* pDevice);
    void                                           Clean();

    MBuffer*                                       GetVertexBuffer() { return &m_vertexBuffer; }
    MBuffer*                                       GetIndexBuffer() { return &m_indexBuffer; }
    [[nodiscard]] const MByte*                     GetVertices() const { return m_vertexData.data(); }
    [[nodiscard]] MByte*                           GetVertices() { return m_vertexData.data(); }
    [[nodiscard]] const std::vector<MByte>&        GetVerticesVector() const { return m_vertexData; }
    [[nodiscard]] const std::vector<MIndicesType>& GetIndicesVector() const { return m_indexData; }
    [[nodiscard]] const MIndicesType*              GetIndices() const
    {
        return reinterpret_cast<const MIndicesType*>(m_indexData.data());
    }
    MIndicesType*                 GetIndices() { return reinterpret_cast<MIndicesType*>(m_indexData.data()); }
    [[nodiscard]] static uint32_t GetIndexStructSize() { return sizeof(MIndicesType); }
    [[nodiscard]] uint32_t        GetVerticesNum() const;
    [[nodiscard]] uint32_t        GetIndicesNum() const;
    [[nodiscard]] uint32_t        GetVerticesSize() const { return static_cast<uint32_t>(m_vertexBuffer.GetSize()); }
    [[nodiscard]] uint32_t        GetIndicesSize() const { return static_cast<uint32_t>(m_indexBuffer.GetSize()); }
    void                          CreateIndices(const uint32_t& nSize, const uint32_t& nIndexSize);
    void                          ResizeIndices(const uint32_t& nSize, const uint32_t& nIndexSize);
    [[nodiscard]] const std::vector<MCluster>& GetClusters() const { return m_clusters; }
    std::vector<MCluster>&                     GetClusters() { return m_clusters; }
    std::vector<MClusterGroup>&                GetClusterGroup() { return m_groups; }
    std::vector<MClusterLodData>&              GetClusterLodData() { return m_lods; }
    std::vector<MClusterPage>&                 GetClusterPages() { return m_clusterPages; }

    [[nodiscard]] virtual size_t               GetAttributeProtectMask() const           = 0;
    [[nodiscard]] virtual std::vector<float>   GetSimplifyWeight() const                 = 0;
    [[nodiscard]] virtual uint32_t             GetVertexStructSize() const               = 0;
    [[nodiscard]] virtual MIMesh*              Clone(const bool& bDynamic = false) const = 0;
    virtual void                               CreateVertices(const uint32_t& nSize)     = 0;
    virtual void                               ResizeVertices(const uint32_t& nSize)     = 0;


protected:
    MBuffer                      m_vertexBuffer;
    MBuffer                      m_indexBuffer;

    std::vector<MByte>           m_vertexData;
    std::vector<MIndicesType>    m_indexData;
    std::vector<MCluster>        m_clusters;
    std::vector<MClusterGroup>   m_groups;
    std::vector<MClusterLodData> m_lods;
    std::vector<MClusterPage>    m_clusterPages;
};

template<class VERTEX_TYPE> class MORTY_API MMesh : public MIMesh
{
public:
    explicit MMesh(const bool& nDynamicMesh = false)
        : MIMesh(nDynamicMesh)
    {}

    ~MMesh() override {}

    [[nodiscard]] MIMesh* Clone(const bool& bDynamic = false) const override
    {
        auto* pNewMesh           = new MMesh<VERTEX_TYPE>(bDynamic);
        pNewMesh->m_vertexBuffer = m_vertexBuffer;
        pNewMesh->m_indexBuffer  = m_indexBuffer;

        return pNewMesh;
    }

public:
    [[nodiscard]] uint32_t GetVertexStructSize() const override { return sizeof(VERTEX_TYPE); }

    void                   CreateVertices(const uint32_t& nSize) override
    {
        if (m_vertexBuffer.GetSize() < nSize * sizeof(VERTEX_TYPE))
        {
            m_vertexBuffer.ReallocMemory(nSize * sizeof(VERTEX_TYPE));
        }
        m_vertexData.resize(nSize * sizeof(VERTEX_TYPE));
    }

    void ResizeVertices(const uint32_t& nSize) override
    {
        if (m_vertexBuffer.GetSize() < nSize * sizeof(VERTEX_TYPE))
        {
            m_vertexBuffer.ReallocMemory(nSize * sizeof(VERTEX_TYPE));
        }
        m_vertexData.resize(nSize * sizeof(VERTEX_TYPE));
    }

    VERTEX_TYPE*         GetVertices() { return reinterpret_cast<VERTEX_TYPE*>(m_vertexData.data()); }

    [[nodiscard]] size_t GetAttributeProtectMask() const override { return AttributeProtectMask<VERTEX_TYPE>(); }
    [[nodiscard]] std::vector<float> GetSimplifyWeight() const override { return SimplifyWeight<VERTEX_TYPE>(); }
};

}// namespace morty