#include "Resource/MMeshResource.h"
#include "Engine/MEngine.h"
#include "Model/MMultiLevelMesh.h"
#include "Flatbuffer/MMeshResource_generated.h"

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

    return static_cast<MMeshResourceData*>(m_resourceData.get())->mesh.get();
}

MIMesh* MMeshResource::GetLevelMesh(const uint32_t unLevel)
{
    if (MRenderGlobal::MESH_LOD_LEVEL_RANGE <= unLevel) return GetMesh();

    if (MIMesh* mesh = GetMesh())
    {
        if (nullptr == m_meshDetailMap)
        {
            m_meshDetailMap = new MMultiLevelMesh();
            m_meshDetailMap->BindMesh(mesh);
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
    const auto                                                  fbObb    = boundsOBB.Serialize(fbb);
    const auto                                                  fbSphere = boundsSphere.Serialize(fbb);
    const auto                                                  fbVertex = fbb.CreateVector(mesh->GetVerticesVector());
    const auto                                                  fbIndex  = fbb.CreateVector(mesh->GetIndicesVector());

    std::vector<flatbuffers::Offset<morty::fbs::MCluster>>      fbClusterArray(mesh->GetClusters().size());
    std::vector<flatbuffers::Offset<morty::fbs::MClusterGroup>> fbGroupArray(mesh->GetClusterGroup().size());
    std::vector<flatbuffers::Offset<morty::fbs::MSlice>>        fbLodArray(mesh->GetClusterLodData().size());
    std::vector<flatbuffers::Offset<morty::fbs::MClusterPage>>  fbPageArray(mesh->GetClusterPages().size());

    std::transform(
            mesh->GetClusters().begin(),
            mesh->GetClusters().end(),
            fbClusterArray.begin(),
            [&fbb](const auto& item) { return item.Serialize(fbb).o; }
    );
    std::transform(
            mesh->GetClusterGroup().begin(),
            mesh->GetClusterGroup().end(),
            fbGroupArray.begin(),
            [&fbb](const auto& item) { return item.Serialize(fbb).o; }
    );
    std::transform(
            mesh->GetClusterLodData().begin(),
            mesh->GetClusterLodData().end(),
            fbLodArray.begin(),
            [&fbb](const auto& item) { return item.Serialize(fbb).o; }
    );
    std::transform(
            mesh->GetClusterPages().begin(),
            mesh->GetClusterPages().end(),
            fbPageArray.begin(),
            [&fbb](const auto& item) { return item.Serialize(fbb).o; }
    );

    const auto                fbClusters     = fbb.CreateVector(fbClusterArray);
    const auto                fbGroups       = fbb.CreateVector(fbGroupArray);
    const auto                fbLods         = fbb.CreateVector(fbLodArray);
    const auto                fbPages        = fbb.CreateVector(fbPageArray);
    const auto                fbGroupLinks = fbb.CreateVector(mesh->GetGroupLinks());

    fbs::MMeshResourceBuilder builder(fbb);

    builder.add_bounds_obb(fbObb.o);
    builder.add_bounds_sphere(fbSphere.o);
    builder.add_vertex_type(static_cast<fbs::MEMeshVertexType>(eVertexType));
    builder.add_vertex(fbVertex.o);
    builder.add_index(fbIndex.o);
    builder.add_cluster(fbClusters.o);
    builder.add_group(fbGroups.o);
    builder.add_lod(fbLods.o);
    builder.add_pages(fbPages.o);
    builder.add_group_links(fbGroupLinks.o);

    return builder.Finish().Union();
}

void MMeshResourceData::Deserialize(const void* pBufferPointer)
{
    const fbs::MMeshResource* fbData = fbs::GetMMeshResource(pBufferPointer);

    eVertexType = static_cast<MEMeshVertexType>(fbData->vertex_type());
    mesh       = MMeshUtil::CreateMeshFromType(eVertexType);

    const auto nVertexNum = static_cast<uint32_t>(fbData->vertex()->size() / mesh->GetVertexStructSize());
    mesh->ResizeVertices(nVertexNum);
    memcpy(mesh->GetVertices(), fbData->vertex()->data(), nVertexNum * mesh->GetVertexStructSize());

    const auto nIndexNum = static_cast<uint32_t>(fbData->index()->size() / sizeof(uint32_t));
    mesh->ResizeIndices(nIndexNum, 1);
    memcpy(mesh->GetIndices(), fbData->index()->data(), nIndexNum * sizeof(uint32_t));

    boundsOBB.Deserialize(fbData->bounds_obb());
    boundsSphere.Deserialize(fbData->bounds_sphere());


    std::vector<MCluster> clusters(fbData->cluster()->size());
    std::transform(fbData->cluster()->begin(), fbData->cluster()->end(), clusters.begin(), [](const auto& item) {
        MCluster cluster;
        cluster.Deserialize(item);
        return cluster;
    });

    std::vector<MClusterGroup> groups(fbData->group()->size());
    std::transform(fbData->group()->begin(), fbData->group()->end(), groups.begin(), [](const auto& item) {
        MClusterGroup group;
        group.Deserialize(item);
        return group;
    });

    std::vector<MClusterLodData> lods(fbData->lod()->size());
    std::transform(fbData->lod()->begin(), fbData->lod()->end(), lods.begin(), [](const auto& item) {
        MClusterLodData lod{};
        lod.Deserialize(item);
        return lod;
    });

    std::vector<MClusterPage> pages(fbData->pages()->size());
    std::transform(fbData->pages()->begin(), fbData->pages()->end(), pages.begin(), [](const auto& item) {
        MClusterPage page{};
        page.Deserialize(item);
        return page;
    });

    mesh->GetClusters()       = std::move(clusters);
    mesh->GetClusterGroup()   = std::move(groups);
    mesh->GetClusterLodData() = std::move(lods);
    mesh->GetClusterPages()   = std::move(pages);

    if (fbData->group_links())
    {
        mesh->GetGroupLinks().assign(
                fbData->group_links()->begin(),
                fbData->group_links()->end()
        );
    }
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
    MRenderSystem* renderSystem = GetEngine()->GetSystem<MRenderSystem>();
    if (MIMesh* mesh = GetMesh()) { mesh->DestroyBuffer(renderSystem->GetDevice()); }
}

void MMeshResource::ResetBounds()
{
    if (MIMesh* mesh = GetMesh())
    {
        auto pMeshData = static_cast<MMeshResourceData*>(m_resourceData.get());
        pMeshData->boundsOBB.SetPoints(
                (const MByte*) mesh->GetVertices(),
                mesh->GetVerticesNum(),
                0,
                mesh->GetVertexStructSize()
        );
        pMeshData->boundsSphere.SetPoints(
                (const MByte*) mesh->GetVertices(),
                mesh->GetVerticesNum(),
                0,
                mesh->GetVertexStructSize()
        );
    }
}
