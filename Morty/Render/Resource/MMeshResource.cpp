#include "Resource/MMeshResource.h"
#include "Engine/MEngine.h"
#include "Flatbuffer/MMeshResource_generated.h"
#include "Model/MMultiLevelMesh.h"

#include "Math/MMath.h"
#include "Mesh/MMesh.h"
#include "Mesh/MVertex.h"
#include "Utility/MFileHelper.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MMeshResource, MResource)


MMeshResource::MMeshResource()
    : m_resourceData(nullptr)
    , m_meshDetailMap(nullptr)
{}

MMeshResource::~MMeshResource()
{
    if (m_meshDetailMap)
    {
        delete m_meshDetailMap;
        m_meshDetailMap = nullptr;
    }

    Clean();
}

MIMesh* MMeshResource::GetMesh() const
{
    if (m_resourceData == nullptr) { return nullptr; }

    return static_cast<MMeshResourceData*>(m_resourceData.get())->pMesh.get();
}

MIMesh* MMeshResource::GetLevelMesh(const uint32_t unLevel)
{
    if (MRenderGlobal::MESH_LOD_LEVEL_RANGE <= unLevel) return GetMesh();

    if (MIMesh* pMesh = GetMesh())
    {
        if (nullptr == m_meshDetailMap)
        {
            m_meshDetailMap = new MMultiLevelMesh();
            m_meshDetailMap->BindMesh(pMesh);
        }

        return m_meshDetailMap->GetLevel(unLevel);
    }

    return nullptr;
}

MEMeshVertexType MMeshResource::GetMeshVertexType() const
{
    if (auto ptr = static_cast<MMeshResourceData*>(m_resourceData.get())) { return ptr->eVertexType; }

    return MEMeshVertexType::Normal;
}

const MBoundsOBB* MMeshResource::GetMeshesDefaultOBB() const
{
    if (auto ptr = static_cast<MMeshResourceData*>(m_resourceData.get())) { return &ptr->boundsOBB; }

    return nullptr;
}

const MBoundsSphere* MMeshResource::GetMeshesDefaultSphere() const
{
    if (auto ptr = static_cast<MMeshResourceData*>(m_resourceData.get())) { return &ptr->boundsSphere; }

    return nullptr;
}

flatbuffers::Offset<void> MMeshResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
    const auto                fbObb    = boundsOBB.Serialize(fbb);
    const auto                fbSphere = boundsSphere.Serialize(fbb);
    const auto                fbVertex = fbb.CreateVector(pMesh->GetVerticesVector());
    const auto                fbIndex  = fbb.CreateVector(pMesh->GetIndicesVector());

    fbs::MMeshResourceBuilder builder(fbb);

    builder.add_bounds_obb(fbObb.o);
    builder.add_bounds_sphere(fbSphere.o);
    builder.add_vertex_type(static_cast<fbs::MEMeshVertexType>(eVertexType));
    builder.add_vertex(fbVertex.o);
    builder.add_index(fbIndex.o);

    return builder.Finish().Union();
    ;
}

void MMeshResourceData::Deserialize(const void* pBufferPointer)
{
    const fbs::MMeshResource* fbData = fbs::GetMMeshResource(pBufferPointer);

    eVertexType = static_cast<MEMeshVertexType>(fbData->vertex_type());
    pMesh       = MMeshUtil::CreateMeshFromType(eVertexType);

    const uint32_t nVertexNum = static_cast<uint32_t>(fbData->vertex()->size() / pMesh->GetVertexStructSize());
    pMesh->ResizeVertices(nVertexNum);
    memcpy(pMesh->GetVertices(), fbData->vertex()->data(), nVertexNum * pMesh->GetVertexStructSize());

    const uint32_t nIndexNum = static_cast<uint32_t>(fbData->index()->size() / sizeof(uint32_t));
    pMesh->ResizeIndices(nIndexNum, 1);
    memcpy(pMesh->GetIndices(), fbData->index()->data(), nIndexNum * sizeof(uint32_t));

    boundsOBB.Deserialize(fbData->bounds_obb());
    boundsSphere.Deserialize(fbData->bounds_sphere());
}

bool MMeshResource::Load(std::unique_ptr<MResourceData>&& pResourceData)
{
    m_resourceData = std::move(pResourceData);

    return true;
}

bool MMeshResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
    auto data     = std::make_unique<MMeshResourceData>(*static_cast<MMeshResourceData*>(m_resourceData.get()));
    pResourceData = std::move(data);
    return true;
}

void MMeshResource::OnDelete() { MResource::OnDelete(); }

void MMeshResource::Clean()
{
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    if (MIMesh* pMesh = GetMesh()) { pMesh->DestroyBuffer(pRenderSystem->GetDevice()); }
}

void MMeshResource::ResetBounds()
{
    if (MIMesh* pMesh = GetMesh())
    {
        auto pMeshData = static_cast<MMeshResourceData*>(m_resourceData.get());
        pMeshData->boundsOBB.SetPoints(
                (const MByte*) pMesh->GetVertices(),
                pMesh->GetVerticesNum(),
                0,
                pMesh->GetVertexStructSize()
        );
        pMeshData->boundsSphere.SetPoints(
                (const MByte*) pMesh->GetVertices(),
                pMesh->GetVerticesNum(),
                0,
                pMesh->GetVertexStructSize()
        );
    }
}
