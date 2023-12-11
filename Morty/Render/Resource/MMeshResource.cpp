#include "Resource/MMeshResource.h"
#include "Engine/MEngine.h"
#include "Flatbuffer/MMeshResource_generated.h"
#include "Model/MMultiLevelMesh.h"

#include "Math/MMath.h"
#include "Render/MMesh.h"
#include "Render/MVertex.h"
#include "Utility/MFileHelper.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"


MORTY_CLASS_IMPLEMENT(MMeshResource, MResource)



MMeshResource::MMeshResource()
	: m_pResourceData(nullptr)
	, m_pMeshDetailMap(nullptr)
{
}

MMeshResource::~MMeshResource()
{
	if (m_pMeshDetailMap)
	{
		delete m_pMeshDetailMap;
		m_pMeshDetailMap = nullptr;
	}

	Clean();
}

MIMesh* MMeshResource::GetMesh() const
{
	if (m_pResourceData == nullptr)
	{
		return nullptr;
	}

	return static_cast<MMeshResourceData*>(m_pResourceData.get())->pMesh.get();
}

MIMesh* MMeshResource::GetLevelMesh(const uint32_t unLevel)
{
	if (MRenderGlobal::MESH_LOD_LEVEL_RANGE <= unLevel)
		return GetMesh();

	if (MIMesh* pMesh = GetMesh())
	{
		if (nullptr == m_pMeshDetailMap)
		{
			m_pMeshDetailMap = new MMultiLevelMesh();
			m_pMeshDetailMap->BindMesh(pMesh);
		}

		return m_pMeshDetailMap->GetLevel(unLevel);
	}

	return nullptr;
}

MEMeshVertexType MMeshResource::GetMeshVertexType() const
{
	if (auto ptr = static_cast<MMeshResourceData*>(m_pResourceData.get()))
	{
		return ptr->eVertexType;
	}

	return MEMeshVertexType::Normal;
}

const MBoundsOBB* MMeshResource::GetMeshesDefaultOBB() const
{
	if (auto ptr = static_cast<MMeshResourceData*>(m_pResourceData.get()))
	{
		return &ptr->boundsOBB;
	}

	return nullptr;
}

const MBoundsSphere* MMeshResource::GetMeshesDefaultSphere() const
{
	if (auto ptr = static_cast<MMeshResourceData*>(m_pResourceData.get()))
	{
		return &ptr->boundsSphere;
	}

	return nullptr;
}

flatbuffers::Offset<void> MMeshResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	const auto fbObb = boundsOBB.Serialize(fbb);
	const auto fbSphere = boundsSphere.Serialize(fbb);
	const auto fbVertex = fbb.CreateVector(pMesh->GetVerticesVector());
	const auto fbIndex = fbb.CreateVector(pMesh->GetIndicesVector());

	mfbs::MMeshResourceBuilder builder(fbb);

	builder.add_bounds_obb(fbObb.o);
	builder.add_bounds_sphere(fbSphere.o);
	builder.add_vertex_type(static_cast<mfbs::MEMeshVertexType>(eVertexType));
	builder.add_vertex(fbVertex.o);
	builder.add_index(fbIndex.o);

	return builder.Finish().Union();;
}

void MMeshResourceData::Deserialize(const void* pBufferPointer)
{
	const mfbs::MMeshResource* fbData = mfbs::GetMMeshResource(pBufferPointer);

	eVertexType = static_cast<MEMeshVertexType>(fbData->vertex_type());
	pMesh = MMeshUtil::CreateMeshFromType(eVertexType);

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
	m_pResourceData = std::move(pResourceData);

	return true;
}

bool MMeshResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
	auto data = std::make_unique<MMeshResourceData>(*static_cast<MMeshResourceData*>(m_pResourceData.get()));
	pResourceData = std::move(data);
	return true;
}

void MMeshResource::OnDelete()
{
	MResource::OnDelete();
}

void MMeshResource::Clean()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	if (MIMesh* pMesh = GetMesh())
	{
		pMesh->DestroyBuffer(pRenderSystem->GetDevice());
	}
}

void MMeshResource::ResetBounds()
{
	if (MIMesh* pMesh = GetMesh())
	{
		auto pMeshData = static_cast<MMeshResourceData*>(m_pResourceData.get());
		pMeshData->boundsOBB.SetPoints((const MByte*)pMesh->GetVertices(), pMesh->GetVerticesNum(), 0, pMesh->GetVertexStructSize());
		pMeshData->boundsSphere.SetPoints((const MByte*)pMesh->GetVertices(), pMesh->GetVerticesNum(), 0, pMesh->GetVertexStructSize());
	}
}
