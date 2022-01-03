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


class MSphereFactory
{
public:

	void operator()(int nLevel = 0);

public:

	std::vector<Vector3> m_vVertex;
	std::vector<uint32_t> m_vIndices;

	uint32_t addMidVertex(uint32_t v1, uint32_t v2);

	std::map<uint32_t, uint32_t> m_tMidIndexCache;
};

uint32_t MSphereFactory::addMidVertex(uint32_t v1, uint32_t v2)
{
	uint32_t key = floor((v1 + v2) * (v1 + v2 + 1) / 2) + (std::min)(v1, v2);

	auto&& findResult = m_tMidIndexCache.find(key);
	if (findResult != m_tMidIndexCache.end())
		return findResult->second;

	m_vVertex.push_back((m_vVertex[v1] + m_vVertex[v2]) * 0.5f);

	m_tMidIndexCache[key] = m_vVertex.size() - 1;
	return m_vVertex.size() - 1;
}

void MSphereFactory::operator()(int nLevel/* = 0*/)
{
	const float t = (1.0f + sqrt(5.0f)) / 2.0f;

	m_vVertex = {
		Vector3(-1.0f, t, 0.0f),
		Vector3(1.0f, t, 0.0f),
		Vector3(-1.0f, -t, 0.0f),
		Vector3(1.0f, -t, 0.0f),

		Vector3(0.0f, -1.0f, t),
		Vector3(0.0f, 1.0f, t),
		Vector3(0.0f, -1.0f, -t),
		Vector3(0.0f, 1.0f, -t),

		Vector3(t, 0.0f, -1.0f),
		Vector3(t, 0.0f, 1.0f),
		Vector3(-t, 0.0f, -1.0f),
		Vector3(-t, 0.0f, 1.0f),
	};

	m_vIndices = {
		0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11,
		1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8,
		3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9,
		4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1,
	};

	m_tMidIndexCache = {};


	for (int i = 0; i < nLevel; ++i)
	{
		std::vector<uint32_t> newIndices(m_vIndices.size() * 4);

		for (int indexIdx = 0; indexIdx < m_vIndices.size(); indexIdx += 3)
		{
			uint32_t& v1 = m_vIndices[indexIdx + 0];
			uint32_t& v2 = m_vIndices[indexIdx + 1];
			uint32_t& v3 = m_vIndices[indexIdx + 2];

			uint32_t a = addMidVertex(v1, v2);
			uint32_t b = addMidVertex(v2, v3);
			uint32_t c = addMidVertex(v3, v1);

			int idx = indexIdx * 4;
			newIndices[idx + 0] = v1; newIndices[idx + 1] = a; newIndices[idx + 2] = c;
			newIndices[idx + 3] = v2; newIndices[idx + 4] = b; newIndices[idx + 5] = a;
			newIndices[idx + 6] = v3; newIndices[idx + 7] = c; newIndices[idx + 8] = b;
			newIndices[idx + 9] = a; newIndices[idx + 10] = b; newIndices[idx + 11] = c;
		}

		m_vIndices.swap(newIndices);
	}
}


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

void MMeshResource::LoadAsPlane(MEMeshVertexType eVertexType/* = MEMeshVertexType::Normal*/, const Vector3& scale/* = Vector3::One */)
{
	m_eVertexType = eVertexType;

	m_pMesh = NewMeshByType(m_eVertexType);
	m_pMesh->ResizeVertices(4);

	/*
		0 -------- 1
		 |        |
		 |        |
		 |        |
		2 -------- 3
	*/
	static const MVertex plane[4] = {
		{ {-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f} },
		{ {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },
		{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} },
		{ {1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f} }
	};

	if (MEMeshVertexType::Normal == eVertexType)
	{
		MVertex* vVertex = (MVertex*)m_pMesh->GetVertices();
		memcpy(m_pMesh->GetVertices(), plane, sizeof(MVertex) * 4);
		for (int i = 0; i < 4; ++i)
		{
			vVertex[i].position.x *= scale.x;
			vVertex[i].position.y *= scale.y;
		}
	}
	else if (MEMeshVertexType::Skeleton == eVertexType)
	{
		for (int i = 0; i < 4; ++i)
		{
			MVertexWithBones* vVertex = (MVertexWithBones*)m_pMesh->GetVertices();
			memcpy(&vVertex[i], &plane[i], sizeof(MVertexWithBones));

			vVertex[i].position.x *= scale.x;
			vVertex[i].position.y *= scale.y;
		}
	}

	static const uint32_t indices[6] = {
		0, 1 ,2, 1, 3, 2,
	};

	m_pMesh->ResizeIndices(2, 3);
	memcpy(m_pMesh->GetIndices(), indices, sizeof(indices));

	m_BoundsOBB.m_matEigVectors = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	m_BoundsOBB.m_v3HalfLength = Vector3(scale.x, scale.y, 0.001f);
	m_BoundsOBB.m_v3MinPoint = -Vector3(-scale.x, -scale.y, -0.001f);
	m_BoundsOBB.m_v3MaxPoint = Vector3(scale.x, scale.y, 0.001f);

	m_BoundsSphere.m_fRadius = scale.Length();
}

void MMeshResource::LoadAsCube(MEMeshVertexType eVertexType/* = MEMeshVertexType::Normal*/)
{
	m_eVertexType = eVertexType;

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
		{0.0f, 1.0f, 0.0f},		//4 top
		{0.0f, -1.0f, 0.0f},	//5 bottom
	};

	static const Vector2 uv[4] = {
		{0.0f, 0.0f},
		{1.0f, 0.0f},
		{0.0f, 1.0f},
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

	if (MEMeshVertexType::Normal == eVertexType)
	{
		m_pMesh->ResizeVertices(24);
		memcpy(m_pMesh->GetVertices(), cube, sizeof(MVertex) * 24);
	}
	else if(MEMeshVertexType::Skeleton == eVertexType)
	{
		m_pMesh->ResizeVertices(24);
		for (int i = 0; i < 24; ++i)
		{
			MVertexWithBones* vVertex = (MVertexWithBones*)m_pMesh->GetVertices();
			memcpy(&vVertex[i], &cube[i], sizeof(MVertexWithBones));
		}
	}

	m_pMesh->ResizeIndices(12, 3);
	memcpy(m_pMesh->GetIndices(), indices, sizeof(indices));

	m_BoundsOBB.m_matEigVectors = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	m_BoundsOBB.m_v3HalfLength = Vector3::One;
	m_BoundsOBB.m_v3MinPoint = -Vector3::One;
	m_BoundsOBB.m_v3MaxPoint = Vector3::One;
	
	m_BoundsSphere.m_fRadius = std::sqrtf(3.0f);
	
}

void MMeshResource::LoadAsSphere(MEMeshVertexType eVertexType/* = MEMeshVertexType::Normal*/)
{
	MSphereFactory sphereFactory;

	sphereFactory(3);

	m_eVertexType = eVertexType;

	m_pMesh = NewMeshByType(m_eVertexType);

	auto& vPoints = sphereFactory.m_vVertex;

	auto& vIndices = sphereFactory.m_vIndices;

	m_pMesh->ResizeVertices(vPoints.size());
	if (eVertexType == MEMeshVertexType::Normal)
	{
		for (int i = 0; i < vPoints.size(); ++i)
		{

			MVertex& vertex = ((MVertex*)m_pMesh->GetVertices())[i];
			vPoints[i].Normalize();

			vertex.position = vPoints[i];
			vertex.normal = vPoints[i];

			Vector3 t1 = vertex.normal.CrossProduct(Vector3::Forward);
			Vector3 t2 = vertex.normal.CrossProduct(Vector3::Up);

			vertex.tangent = t1.Length() > t2.Length() ? t1 : t2;
			vertex.bitangent = vertex.normal.CrossProduct(vertex.tangent);

			float xzLength = sqrt(vPoints[i].x * vPoints[i].x + vPoints[i].z * vPoints[i].z);
			vertex.texCoords.x = acos(vPoints[i].x / xzLength) / M_PI;
			vertex.texCoords.y = acos(vPoints[i].y) / M_PI;
		}
	}
	else if (eVertexType == MEMeshVertexType::Skeleton)
	{
		for (int i = 0; i < vPoints.size(); ++i)
		{

			MVertexWithBones& vertex = ((MVertexWithBones*)m_pMesh->GetVertices())[i];
			vPoints[i].Normalize();

			vertex.position = vPoints[i];
			vertex.normal = vPoints[i];

			Vector3 t1 = vertex.normal.CrossProduct(Vector3::Forward);
			Vector3 t2 = vertex.normal.CrossProduct(Vector3::Up);

			vertex.tangent = t1.Length() > t2.Length() ? t1 : t2;
			vertex.bitangent = vertex.normal.CrossProduct(vertex.tangent);

			float xzLength = sqrt(vPoints[i].x * vPoints[i].x + vPoints[i].z * vPoints[i].z);
			vertex.texCoords.x = acos(vPoints[i].x / xzLength) / M_PI;
			vertex.texCoords.y = acos(vPoints[i].y) / M_PI;
		}
	}

	m_pMesh->ResizeIndices(vIndices.size(), 1);
	memcpy(m_pMesh->GetIndices(), vIndices.data(), sizeof(uint32_t) * vIndices.size());

	m_BoundsOBB.m_matEigVectors = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	m_BoundsOBB.m_v3HalfLength = Vector3(1.0f, 1.0f, 1.0f);
	m_BoundsOBB.m_v3MinPoint = Vector3(-1.0f, -1.0f, -1.0f);
	m_BoundsOBB.m_v3MaxPoint = Vector3(1.0f, 1.0f, 1.0f);

	m_BoundsSphere.m_fRadius = std::sqrtf(1.0f);
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
