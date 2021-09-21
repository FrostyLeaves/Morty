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

void MMeshResource::LoadAsCube()
{
	m_eVertexType = MEMeshVertexType::Normal;

	/*
	Vector3 position;
	Vector3 normal;
	Vector3 tangent;
	Vector3 bitangent;
	Vector2 texCoords;
	*/

	/*
     
	   5 ___________4
	    /|         /|
	   / |        / |
	0 /__|_______/1 |
	 |   |_ _ _ |_ _|
	 |  / 7     |  / 6
	 | /        | /
	 |/_________|/
	 2           3

	*/
	static const Vector3 position[8] = {
		{-1.0f, 1.0f, -1.0f},	//front left top
		{1.0f, 1.0f, -1.0f},	//front right top
		{-1.0f, -1.0f, -1.0f},	//front left bottom
		{1.0f, -1.0f, -1.0f},	//front right bottom
		{1.0f, 1.0f, 1.0f},		//back right top
		{-1.0f, 1.0f, 1.0f},	//back left top
		{1.0f, -1.0f, 1.0f},	//back right bottom
		{-1.0f, -1.0f, 1.0f},	//back left bottom
	};

	static const Vector3 normal[6] = {
		{0.0f, 0.0f, 1.0f},		//0 forward
		{0.0f, 0.0f, -1.0f},	//1 back
		{-1.0f, 0.0f, 0.0f},	//2 left
		{1.0f, 0.0f, 0.0f},		//3 right
		{0.0f, 0.0f, 1.0f},		//4 top
		{0.0f, 0.0f, -1.0f},	//5 bottom
	};

	static const Vector2 uv[4] = {
		{0.0f, 0.0f},
		{0.0f, 1.0f},
		{1.0f, 0.0f},
		{1.0f, 1.0f},
	};

	static const MVertex cube[24] = {
		//front
		{ position[0], normal[1], normal[3], normal[5], uv[0] },
		{ position[1], normal[1], normal[3], normal[5], uv[1] },
		{ position[2], normal[1], normal[3], normal[5], uv[2] },
		{ position[3], normal[1], normal[3], normal[5], uv[3] },

		//back
		{ position[4], normal[0], normal[2], normal[5], uv[0] },
		{ position[5], normal[0], normal[2], normal[5], uv[1] },
		{ position[6], normal[0], normal[2], normal[5], uv[2] },
		{ position[7], normal[0], normal[2], normal[5], uv[3] },

		//left
		{ position[5], normal[2], normal[0], normal[5], uv[0] },
		{ position[0], normal[2], normal[0], normal[5], uv[1] },
		{ position[7], normal[2], normal[0], normal[5], uv[2] },
		{ position[2], normal[2], normal[0], normal[5], uv[3] },

		//right
		{ position[1], normal[3], normal[0], normal[5], uv[0] },
		{ position[4], normal[3], normal[0], normal[5], uv[1] },
		{ position[3], normal[3], normal[0], normal[5], uv[2] },
		{ position[6], normal[3], normal[0], normal[5], uv[3] },

		//top
		{ position[5], normal[4], normal[3], normal[1], uv[0] },
		{ position[4], normal[4], normal[3], normal[1], uv[1] },
		{ position[0], normal[4], normal[3], normal[1], uv[2] },
		{ position[1], normal[4], normal[3], normal[1], uv[3] },

		//bottom
		{ position[6], normal[5], normal[2], normal[1], uv[0] },
		{ position[7], normal[5], normal[2], normal[1], uv[1] },
		{ position[3], normal[5], normal[2], normal[1], uv[2] },
		{ position[2], normal[5], normal[2], normal[1], uv[3] },
	};

	static const uint32_t indices[36] = {
		0, 1 ,2, 1, 3, 2,
		4, 5, 6, 5, 7, 6,
		8, 9, 10, 9, 11, 10,
		12, 13, 14, 13, 15, 14,
		16, 17, 18, 17, 19, 18,
		20, 21, 22, 21, 23, 22
	};

	m_pMesh = NewMeshByType(m_eVertexType);

	m_pMesh->ResizeVertices(24);
	memcpy(m_pMesh->GetVertices(), cube, sizeof(cube));

	m_pMesh->ResizeIndices(12, 3);
	memcpy(m_pMesh->GetIndices(), indices, sizeof(indices));


	
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
