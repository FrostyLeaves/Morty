#include "Resource/MMeshResourceUtil.h"

#include "Math/MMath.h"
#include "Mesh/MMeshUtil.h"
#include "Mesh/MVertex.h"
#include "Resource/MMeshResource.h"

using namespace morty;

std::unique_ptr<MResourceData> MMeshResourceUtil::CreatePlane(MEMeshVertexType eVertexType)
{
    auto pMeshData                       = std::make_unique<MMeshResourceData>();
    pMeshData->eVertexType               = eVertexType;
    pMeshData->pMesh                     = MMeshUtil::CreatePlane(eVertexType);
    pMeshData->boundsOBB.m_matEigVectors = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    pMeshData->boundsOBB.m_halfLength    = Vector3(1.0f, 1.0f, 0.001f);
    pMeshData->boundsOBB.m_minPoint      = -Vector3(-1.0f, -1.0f, -0.001f);
    pMeshData->boundsOBB.m_maxPoint      = Vector3(1.0f, 1.0f, 0.001f);

    pMeshData->boundsSphere.m_radius = 1.0f;

    return pMeshData;
}

std::unique_ptr<MResourceData> MMeshResourceUtil::CreateCube(MEMeshVertexType eVertexType)
{
    auto pMeshData = std::make_unique<MMeshResourceData>();

    pMeshData->eVertexType               = eVertexType;
    pMeshData->pMesh                     = MMeshUtil::CreateCube(eVertexType);
    pMeshData->boundsOBB.m_matEigVectors = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    pMeshData->boundsOBB.m_halfLength    = Vector3::One;
    pMeshData->boundsOBB.m_minPoint      = -Vector3::One;
    pMeshData->boundsOBB.m_maxPoint      = Vector3::One;

    pMeshData->boundsSphere.m_radius = sqrtf(3.0f);

    return pMeshData;
}

std::unique_ptr<MResourceData> MMeshResourceUtil::CreateSphere(MEMeshVertexType eVertexType)
{
    auto pMeshData = std::make_unique<MMeshResourceData>();

    pMeshData->eVertexType               = eVertexType;
    pMeshData->pMesh                     = MMeshUtil::CreateSphere(eVertexType);
    pMeshData->boundsOBB.m_matEigVectors = Matrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    pMeshData->boundsOBB.m_halfLength    = Vector3(1.0f, 1.0f, 1.0f);
    pMeshData->boundsOBB.m_minPoint      = Vector3(-1.0f, -1.0f, -1.0f);
    pMeshData->boundsOBB.m_maxPoint      = Vector3(1.0f, 1.0f, 1.0f);

    pMeshData->boundsSphere.m_radius = sqrtf(1.0f);

    return pMeshData;
}
