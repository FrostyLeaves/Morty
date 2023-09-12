#include "Resource/MMeshResourceUtil.h"

#include "Math/MMath.h"
#include "Mesh/MMeshUtil.h"
#include "Render/MVertex.h"
#include "Resource/MMeshResource.h"

std::unique_ptr<MResourceData> MMeshResourceUtil::CreatePlane(MEMeshVertexType eVertexType)
{
	auto pMeshData = std::make_unique<MMeshResourceData>();
	pMeshData->eVertexType = eVertexType;
	pMeshData->pMesh = MMeshUtil::CreatePlane(eVertexType);
	pMeshData->boundsOBB.m_matEigVectors = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	pMeshData->boundsOBB.m_v3HalfLength = Vector3(1.0f, 1.0f, 0.001f);
	pMeshData->boundsOBB.m_v3MinPoint = -Vector3(-1.0f, -1.0f, -0.001f);
	pMeshData->boundsOBB.m_v3MaxPoint = Vector3(1.0f, 1.0f, 0.001f);

	pMeshData->boundsSphere.m_fRadius = 1.0f;

	return pMeshData;
}

std::unique_ptr<MResourceData> MMeshResourceUtil::CreateCube(MEMeshVertexType eVertexType)
{
	auto pMeshData = std::make_unique<MMeshResourceData>();

	pMeshData->eVertexType = eVertexType;
	pMeshData->pMesh = MMeshUtil::CreateCube(eVertexType);
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

	pMeshData->eVertexType = eVertexType;
	pMeshData->pMesh = MMeshUtil::CreateSphere(eVertexType);
	pMeshData->boundsOBB.m_matEigVectors = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	pMeshData->boundsOBB.m_v3HalfLength = Vector3(1.0f, 1.0f, 1.0f);
	pMeshData->boundsOBB.m_v3MinPoint = Vector3(-1.0f, -1.0f, -1.0f);
	pMeshData->boundsOBB.m_v3MaxPoint = Vector3(1.0f, 1.0f, 1.0f);

	pMeshData->boundsSphere.m_fRadius = std::sqrtf(1.0f);

	return pMeshData;
}
