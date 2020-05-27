#include "MSkinnedMeshInstance.h"
#include "MScene.h"
#include "MMaterial.h"
#include "Model/MModelMeshStruct.h"

#include "MBounds.h"

M_OBJECT_IMPLEMENT(MSkinnedMeshInstance, MIModelMeshInstance)

MSkinnedMeshInstance::MSkinnedMeshInstance()
	: MIModelMeshInstance()
	, m_pMesh(nullptr)
	, m_Material()
	, m_pSkeletonInstance(nullptr)
	, m_bBoundsAABBDirty(true)
	, m_bBoundsSphereDirty(true)
{
	
}

MSkinnedMeshInstance::~MSkinnedMeshInstance()
{

}

void MSkinnedMeshInstance::SetMaterial(MMaterial* pMaterial)
{
	m_Material.SetResource(pMaterial);
}

MMaterial* MSkinnedMeshInstance::GetMaterial()
{
	return dynamic_cast<MMaterial*>(m_Material.GetResource());
}

MBoundsAABB* MSkinnedMeshInstance::GetBoundsAABB()
{
	//TODO get the Bounding Box by skeleton instance.

	if (m_bBoundsAABBDirty)
	{
		Matrix4 matWorldTrans = GetWorldTransform();
		Vector3 v3Position = GetWorldPosition();
		m_BoundsAABB.SetBoundsOBB(v3Position, matWorldTrans, *m_pMesh->GetMeshesDefaultOBB());
		m_bBoundsAABBDirty = false;
	}

	return &m_BoundsAABB;
}

MBoundsSphere* MSkinnedMeshInstance::GetBoundsSphere()
{
	if (m_bBoundsSphereDirty)
	{
		m_BoundsSphere = *m_pMesh->GetMeshesDefaultSphere();

		m_BoundsSphere.m_v3CenterPoint = GetWorldPosition();

		Vector3 v3Scale = GetScale();
		float fMaxScale = v3Scale.x;
		if (fMaxScale < v3Scale.y) fMaxScale = v3Scale.y;
		if (fMaxScale < v3Scale.z) fMaxScale = v3Scale.z;

		m_BoundsSphere.m_fRadius *= fMaxScale;

		m_bBoundsSphereDirty = false;
	}

	return &m_BoundsSphere;
}

void MSkinnedMeshInstance::SetMeshData(MModelMeshStruct* pMeshData)
{
	m_pMesh = pMeshData;
	
	MMaterial* pMaterial = dynamic_cast<MMaterial*>(pMeshData->GetDefaultMaterial());
	SetMaterial(pMaterial);
}

MIMesh* MSkinnedMeshInstance::GetMesh(const unsigned int& unDetailLevel)
{
	if (unDetailLevel == MMESH_LOD_LEVEL_RANGE)
		return m_pMesh->GetMesh();
	else return m_pMesh->GetLevelMesh(unDetailLevel);
}

void MSkinnedMeshInstance::OnDelete()
{
	SetMaterial(nullptr);

	Super::OnDelete();
}

void MSkinnedMeshInstance::UpdateSkeletonBoundsOBB()
{
	
}

void MSkinnedMeshInstance::WorldTransformDirty()
{
	MIMeshInstance::WorldTransformDirty();

	m_bBoundsAABBDirty = true;
}

void MSkinnedMeshInstance::LocalTransformDirty()
{
	MIMeshInstance::LocalTransformDirty();

	m_bBoundsAABBDirty = true;
}
