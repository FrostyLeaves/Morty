#include "MStaticMeshInstance.h"
#include "Model/MModelResource.h"
#include "MScene.h"
#include "MMaterial.h"

#include "Model/MModelResource.h"
#include "Model/MModelMeshStruct.h"

#include "MBounds.h"

MTypeIdentifierImplement(MStaticMeshInstance, MIModelMeshInstance)

MStaticMeshInstance::MStaticMeshInstance()
	: MIModelMeshInstance()
	, m_pMesh(nullptr)
	, m_pMaterial(nullptr)
	, m_bBoundsAABBDirty(true)
	, m_bBoundsSphereDirty(true)
{

}

MStaticMeshInstance::~MStaticMeshInstance()
{
}

void MStaticMeshInstance::SetMaterial(MMaterial* pMaterial)
{
	if (m_pScene)
		m_pScene->CancelRecordMeshInstance(this);

	if (m_pMaterial)
		m_pMaterial->SubRef();

	if(m_pMaterial = pMaterial)
		m_pMaterial->AddRef();

	if(m_pScene)
		m_pScene->RecordMeshInstance(this);
}
 
MBoundsAABB* MStaticMeshInstance::GetBoundsAABB()
{
	if (m_bBoundsAABBDirty)
	{
		Matrix4 matWorldTrans = GetWorldTransform();
		Vector3 v3Position = GetWorldPosition();
		m_BoundsAABB.SetBoundsOBB(v3Position, matWorldTrans, *m_pMesh->GetMeshesDefaultOBB());
		m_bBoundsAABBDirty = false;
	}

	return &m_BoundsAABB;
}

MBoundsSphere* MStaticMeshInstance::GetBoundsSphere()
{
	if (m_bBoundsSphereDirty)
	{
		m_BoundsSphere = *m_pMesh->GetMeshesDefaultSphere();

		m_BoundsSphere.m_v3CenterPoint += GetWorldPosition();

		Vector3 v3Scale = GetScale();
		float fMaxScale = v3Scale.x;
		if (fMaxScale < v3Scale.y) fMaxScale = v3Scale.y;
		if (fMaxScale < v3Scale.z) fMaxScale = v3Scale.z;

		m_BoundsSphere.m_fRadius *= fMaxScale;

		m_bBoundsSphereDirty = false;
	}

	return &m_BoundsSphere;
}

void MStaticMeshInstance::SetMeshData(MModelMeshStruct* pMeshData)
{
	m_pMesh = pMeshData;
	SetMaterial(pMeshData->GetDefaultMaterial());
}

MIMesh* MStaticMeshInstance::GetMesh(const unsigned int& unDetailLevel)
{
	if (unDetailLevel == MMESH_LOD_LEVEL_RANGE)
		return m_pMesh->GetMesh();
	else return m_pMesh->GetLevelMesh(unDetailLevel);
}

MIMesh* MStaticMeshInstance::GetMesh()
{
	return m_pMesh->GetMesh();
}

void MStaticMeshInstance::WorldTransformDirty()
{
	MIMeshInstance::WorldTransformDirty();

	m_bBoundsAABBDirty = true;
}

void MStaticMeshInstance::LocalTransformDirty()
{
	MIMeshInstance::LocalTransformDirty();

	m_bBoundsAABBDirty = true;
	m_bBoundsSphereDirty = true;
}
