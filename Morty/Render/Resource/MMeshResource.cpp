#include "MMeshResource.h"
#include "MModelResource.h"
#include "MEngine.h"
#include "MMultiLevelMesh.h"

#include "MJson.h"
#include "MVertex.h"
#include "MVariant.h"
#include "MFileHelper.h"

#include "MRenderSystem.h"
#include "MResourceSystem.h"

MORTY_CLASS_IMPLEMENT(MMeshResource, MResource)

MMeshResource::MMeshResource()
	: m_eVertexType(MEMeshVertexType::Normal)
	, m_pMesh(nullptr)
	, m_BoundsOBB()
	, m_BoundsSphere()
	, m_MaterialKeeper()
	, m_SkeletonKeeper()
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
	if (MGlobal::MESH_LOD_LEVEL_RANGE <= unLevel)
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

bool MMeshResource::Load(const MString& strResourcePath)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	Clean();

	MMortyFileFormat format;
	if (false == MFileHelper::ReadFormatFile(strResourcePath, format))
		return false;

	MVariant headerVar;
	if (false == MJson::JsonToMVariant(format.m_strHead, headerVar))
		return false;

	MStruct* pHeader = headerVar.GetStruct();
	if (!pHeader) return false;

	int nVertexType = 0;
	if (!pHeader->FindMember<int>("t", nVertexType))
		return false;
	m_eVertexType = (MEMeshVertexType)nVertexType;

	int nVertexBegin, nVertexEnd, nVertexNum;
	int nIndexBegin, nIndexEnd, nIndexNum;
	if (!pHeader->FindMember<int>("v1", nVertexBegin))
		return false;
	if (!pHeader->FindMember<int>("v2", nVertexNum))
		return false;
	if (!pHeader->FindMember<int>("v3", nVertexEnd))
		return false;
	if (!pHeader->FindMember<int>("i1", nIndexBegin))
		return false;
	if (!pHeader->FindMember<int>("i2", nIndexNum))
		return false;
	if (!pHeader->FindMember<int>("i3", nIndexEnd))
		return false;

	m_pMesh = NewMeshByType(m_eVertexType);

	if (m_pMesh->GetVertexStructSize() * nVertexNum != nVertexEnd - nVertexBegin)
		return false;

	if (sizeof(uint32_t) * nIndexNum != nIndexEnd - nIndexBegin)
		return false;

	m_pMesh->ResizeVertices(nVertexNum);
	memcpy(m_pMesh->GetVertices(), format.m_vBody[0].pData + nVertexBegin, nVertexEnd - nVertexBegin);

	m_pMesh->ResizeIndices(nIndexNum, 1);
	memcpy(m_pMesh->GetIndices(), format.m_vBody[0].pData + nIndexBegin, nIndexEnd - nIndexBegin);

	pHeader->FindMember<MString>("n", m_strName);

	if (MStruct* pBoundsObb = pHeader->FindMember<MStruct>("obb"))
	{
		m_BoundsOBB.ReadFromStruct(*pBoundsObb);
	}

	if (MStruct* pBoundsSphere = pHeader->FindMember<MStruct>("sph"))
	{
		m_BoundsSphere.ReadFromStruct(*pBoundsSphere);
	}

	if (MString* pSkeleton = pHeader->FindMember<MString>("ske"))
	{
		if (!pSkeleton->empty())
		{
			MResource* pResource = pResourceSystem->LoadResource(*pSkeleton);
			m_SkeletonKeeper.SetResource(pResource);
		}
	}

	if (MString* pMaterial = pHeader->FindMember<MString>("mat"))
	{
		MResource* pResource = pResourceSystem->LoadResource(*pMaterial);
		m_MaterialKeeper.SetResource(pResource);
	}

	return true;
}

bool MMeshResource::SaveTo(const MString& strResourcePath)
{
	MVariant headerVar = MStruct();
	MStruct* pHeader = headerVar.GetStruct();

	pHeader->AppendMVariant("t", (int)m_eVertexType);

	int nVertexBegin = 0;
	int nVertexNum = m_pMesh->GetVerticesLength();
	int nVertexEnd = nVertexBegin + nVertexNum * m_pMesh->GetVertexStructSize();

	int nIndexBegin = nVertexEnd;
	int nIndexNum = m_pMesh->GetIndicesLength();
	int nIndexEnd = nIndexBegin + nIndexNum * sizeof(uint32_t);

	pHeader->AppendMVariant("v1", nVertexBegin);
	pHeader->AppendMVariant("v2", nVertexNum);
	pHeader->AppendMVariant("v3", nVertexEnd);

	pHeader->AppendMVariant("i1", nIndexBegin);
	pHeader->AppendMVariant("i2", nIndexNum);
	pHeader->AppendMVariant("i3", nIndexEnd);


	pHeader->AppendMVariant("n", m_strName);

	if(MStruct* pObbSrt = pHeader->AppendMVariant<MStruct>("obb"))
		m_BoundsOBB.WriteToStruct(*pObbSrt);
	
	if (MStruct* pSphSrt = pHeader->AppendMVariant<MStruct>("sph"))
		m_BoundsSphere.WriteToStruct(*pSphSrt);

	if (MString* pSkeleton = pHeader->AppendMVariant<MString>("ske"))
	{
		*pSkeleton = m_SkeletonKeeper.GetResourcePath();
	}

	if (MString* pMaterial = pHeader->AppendMVariant<MString>("mat"))
	{
		*pMaterial = m_MaterialKeeper.GetResourcePath();
	}

	MMortyFileFormat format;
	MJson::MVariantToJson(headerVar, format.m_strHead);

	format.PushBackBody(m_pMesh->GetVertices(), nVertexEnd - nVertexBegin);
	format.PushBackBody(m_pMesh->GetIndices(), nIndexEnd - nIndexBegin);

	return MFileHelper::WriteFormatFile(strResourcePath, format);
}

void MMeshResource::OnDelete()
{
	m_MaterialKeeper.SetResource(nullptr);

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
	if (m_pMesh)
	{
		m_BoundsOBB.SetPoints((const MByte*)m_pMesh->GetVertices(), m_pMesh->GetVerticesLength(), 0, m_pMesh->GetVertexStructSize());
		m_BoundsSphere.SetPoints((const MByte*)m_pMesh->GetVertices(), m_pMesh->GetVerticesLength(), 0, m_pMesh->GetVertexStructSize());
	}
}

MIMesh* MMeshResource::NewMeshByType(const MEMeshVertexType& eType)
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
