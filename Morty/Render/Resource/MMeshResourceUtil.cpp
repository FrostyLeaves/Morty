﻿#include "Resource/MMeshResourceUtil.h"

#include "Math/MMath.h"
#include "Render/MVertex.h"
#include "Resource/MMeshResource.h"

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

	auto findResult = m_tMidIndexCache.find(key);
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

std::unique_ptr<MResourceData> MMeshResourceUtil::CreatePlane(MEMeshVertexType eVertexType, const Vector3& scale)
{
	auto pMeshData = std::make_unique<MMeshResourceData>();
	pMeshData->eVertexType = eVertexType;

	pMeshData->pMesh = MMeshResource::CreateMeshFromType(pMeshData->eVertexType);
	pMeshData->pMesh->ResizeVertices(4);

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
		MVertex* vVertex = (MVertex*)pMeshData->pMesh->GetVertices();
		memcpy(pMeshData->pMesh->GetVertices(), plane, sizeof(MVertex) * 4);
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
			MVertexWithBones* vVertex = (MVertexWithBones*)pMeshData->pMesh->GetVertices();
			memcpy(&vVertex[i], &plane[i], sizeof(MVertexWithBones));

			vVertex[i].position.x *= scale.x;
			vVertex[i].position.y *= scale.y;
		}
	}

	static const uint32_t indices[6] = {
		0, 1 ,2, 1, 3, 2,
	};

	pMeshData->pMesh->ResizeIndices(2, 3);
	memcpy(pMeshData->pMesh->GetIndices(), indices, sizeof(indices));

	pMeshData->boundsOBB.m_matEigVectors = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	pMeshData->boundsOBB.m_v3HalfLength = Vector3(scale.x, scale.y, 0.001f);
	pMeshData->boundsOBB.m_v3MinPoint = -Vector3(-scale.x, -scale.y, -0.001f);
	pMeshData->boundsOBB.m_v3MaxPoint = Vector3(scale.x, scale.y, 0.001f);

	pMeshData->boundsSphere.m_fRadius = scale.Length();

	return pMeshData;
}

std::unique_ptr<MResourceData> MMeshResourceUtil::CreateCube(MEMeshVertexType eVertexType)
{
	auto pMeshData = std::make_unique<MMeshResourceData>();

	pMeshData->eVertexType = eVertexType;

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

	pMeshData->pMesh = MMeshResource::CreateMeshFromType(pMeshData->eVertexType);

	if (MEMeshVertexType::Normal == eVertexType)
	{
		pMeshData->pMesh->ResizeVertices(24);
		memcpy(pMeshData->pMesh->GetVertices(), cube, sizeof(MVertex) * 24);
	}
	else if (MEMeshVertexType::Skeleton == eVertexType)
	{
		pMeshData->pMesh->ResizeVertices(24);
		for (int i = 0; i < 24; ++i)
		{
			MVertexWithBones* vVertex = (MVertexWithBones*)pMeshData->pMesh->GetVertices();
			memcpy(&vVertex[i], &cube[i], sizeof(MVertexWithBones));
		}
	}

	pMeshData->pMesh->ResizeIndices(12, 3);
	memcpy(pMeshData->pMesh->GetIndices(), indices, sizeof(indices));

	pMeshData->boundsOBB.m_matEigVectors = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	pMeshData->boundsOBB.m_v3HalfLength = Vector3::One;
	pMeshData->boundsOBB.m_v3MinPoint = -Vector3::One;
	pMeshData->boundsOBB.m_v3MaxPoint = Vector3::One;

	pMeshData->boundsSphere.m_fRadius = std::sqrtf(3.0f);

	return pMeshData;
}

std::unique_ptr<MResourceData> MMeshResourceUtil::CreateSphere(MEMeshVertexType eVertexType)
{
	auto pMeshData = std::make_unique<MMeshResourceData>();

	MSphereFactory sphereFactory;

	sphereFactory(3);

	pMeshData->eVertexType = eVertexType;

	pMeshData->pMesh = MMeshResource::CreateMeshFromType(pMeshData->eVertexType);

	auto& vPoints = sphereFactory.m_vVertex;

	auto& vIndices = sphereFactory.m_vIndices;

	pMeshData->pMesh->ResizeVertices(vPoints.size());

	if (eVertexType == MEMeshVertexType::Normal)
	{
		for (int i = 0; i < vPoints.size(); ++i)
		{

			MVertex& vertex = ((MVertex*)pMeshData->pMesh->GetVertices())[i];
			vPoints[i].Normalize();

			vertex.position = vPoints[i];
			vertex.normal = vPoints[i];

			Vector3 sph = MMath::ConvertToSphericalCoord(vertex.position);

			const float& fLength = sph.x;
			const float& theta = sph.y;
			const float& phi = sph.z;

			float theta_bitangent = theta + M_PI * 0.5f;
			float phi_bitangent = phi;
			if (theta_bitangent >= M_PI)
			{
				theta_bitangent = M_PI * 2.0f - theta_bitangent;
				phi_bitangent = fmodf(phi_bitangent + M_PI, M_PI * 2.0f);
			}

			vertex.bitangent = MMath::ConvertFormSphericalCoord(Vector3(fLength, theta_bitangent, phi_bitangent));
			vertex.bitangent.Normalize();

			vertex.tangent = vertex.normal.CrossProduct(vertex.bitangent);
			vertex.tangent.Normalize();




			vertex.texCoords.x = phi / (M_PI * 2);
			vertex.texCoords.y = theta / (M_PI);
		}
	}
	else if (eVertexType == MEMeshVertexType::Skeleton)
	{
		for (int i = 0; i < vPoints.size(); ++i)
		{

			MVertexWithBones& vertex = ((MVertexWithBones*)pMeshData->pMesh->GetVertices())[i];
			vPoints[i].Normalize();

			vertex.position = vPoints[i];
			vertex.normal = vPoints[i];

			Vector3 t1 = vertex.normal.CrossProduct(Vector3::Forward);
			Vector3 t2 = vertex.normal.CrossProduct(Vector3::Up);

			vertex.tangent = t1.Length() > t2.Length() ? t1 : t2;
			vertex.bitangent = vertex.normal.CrossProduct(vertex.tangent);

			float xzLength = sqrt(vPoints[i].x * vPoints[i].x + vPoints[i].z * vPoints[i].z);
			vertex.texCoords.x = 1.0f - acos(vPoints[i].x / xzLength) / M_PI;
			vertex.texCoords.y = acos(vPoints[i].y) / M_PI;
		}
	}

	pMeshData->pMesh->ResizeIndices(vIndices.size(), 1);
	memcpy(pMeshData->pMesh->GetIndices(), vIndices.data(), sizeof(uint32_t) * vIndices.size());

	pMeshData->boundsOBB.m_matEigVectors = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	pMeshData->boundsOBB.m_v3HalfLength = Vector3(1.0f, 1.0f, 1.0f);
	pMeshData->boundsOBB.m_v3MinPoint = Vector3(-1.0f, -1.0f, -1.0f);
	pMeshData->boundsOBB.m_v3MaxPoint = Vector3(1.0f, 1.0f, 1.0f);

	pMeshData->boundsSphere.m_fRadius = std::sqrtf(1.0f);

	return pMeshData;
}