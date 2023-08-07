#include "Resource/MMeshResource.h"
#include "Engine/MEngine.h"
#include "Flatbuffer/MMeshResource_generated.h"
#include "Model/MMultiLevelMesh.h"

#include "Math/MMath.h"
#include "Render/MVertex.h"
#include "Utility/MFileHelper.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"


MORTY_CLASS_IMPLEMENT(MMeshResource, MResource)



MMeshResource::MMeshResource()
	: m_pMesh(nullptr)
	, m_pResourceData(nullptr)
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

MIMesh* MMeshResource::GetLevelMesh(const uint32_t unLevel)
{
	if (MRenderGlobal::MESH_LOD_LEVEL_RANGE <= unLevel)
		return m_pMesh;

	if (m_pMesh)
	{
		if (nullptr == m_pMeshDetailMap)
		{
			m_pMeshDetailMap = new MMultiLevelMesh();
			m_pMeshDetailMap->BindMesh(m_pMesh);
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
	pMesh = MMeshResource::CreateMeshFromType(eVertexType);

	const size_t nVertexNum = fbData->vertex()->size() / pMesh->GetVertexStructSize();
	pMesh->ResizeVertices(nVertexNum);
	memcpy(pMesh->GetVertices(), fbData->vertex()->data(), nVertexNum * pMesh->GetVertexStructSize());

	const size_t nIndexNum = fbData->index()->size() / sizeof(uint32_t);
	pMesh->ResizeIndices(nIndexNum, 1);
	memcpy(pMesh->GetIndices(), fbData->index()->data(), nIndexNum * sizeof(uint32_t));

	boundsOBB.Deserialize(fbData->bounds_obb());
	boundsSphere.Deserialize(fbData->bounds_sphere());
}

bool MMeshResource::Load(std::unique_ptr<MResourceData>&& pResourceData)
{
	auto pMeshData = static_cast<MMeshResourceData*>(pResourceData.get());
	
	m_pMesh = pMeshData->pMesh;

	m_pResourceData = std::move(pResourceData);

	return true;
}

bool MMeshResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
	pResourceData = std::make_unique<MMeshResourceData>(*static_cast<MMeshResourceData*>(m_pResourceData.get()));
	return true;
}

void MMeshResource::OnDelete()
{
	MResource::OnDelete();
}

void MMeshResource::Clean()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	if (m_pMesh)
	{
		m_pMesh->DestroyBuffer(pRenderSystem->GetDevice());

		delete m_pMesh;
		m_pMesh = nullptr;
	}
}

void MMeshResource::ResetBounds()
{
	if (m_pMesh && m_pResourceData)
	{
		auto pMeshData = static_cast<MMeshResourceData*>(m_pResourceData.get());
		pMeshData->boundsOBB.SetPoints((const MByte*)m_pMesh->GetVertices(), m_pMesh->GetVerticesNum(), 0, m_pMesh->GetVertexStructSize());
		pMeshData->boundsSphere.SetPoints((const MByte*)m_pMesh->GetVertices(), m_pMesh->GetVerticesNum(), 0, m_pMesh->GetVertexStructSize());
	}
}

MIMesh* MMeshResource::CreateMeshFromType(const MEMeshVertexType& eType)
{
	switch (eType)
	{
	case MEMeshVertexType::Normal:
		return new MMesh<MVertex>();
		break;

	case MEMeshVertexType::Skeleton:
		return new MMesh<MVertexWithBones>();
		break;

	default:
		break;
	}

	return nullptr;
}
