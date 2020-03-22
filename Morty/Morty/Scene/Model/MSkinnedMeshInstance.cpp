#include "MSkinnedMeshInstance.h"
#include "MScene.h"
#include "MMaterial.h"
#include "MModelMeshData.h"

#include "MBounds.h"

MTypeIdentifierImplement(MSkinnedMeshInstance, MIModelMeshInstance)

MSkinnedMeshInstance::MSkinnedMeshInstance()
	: MIModelMeshInstance()
	, m_pMesh(nullptr)
	, m_pMaterial(nullptr)
	, m_pSkeletonInstance(nullptr)
	, m_pBoundsAABB(nullptr)
	, m_bBoundsAABBDirty(true)
{

}

MSkinnedMeshInstance::~MSkinnedMeshInstance()
{

}

void MSkinnedMeshInstance::SetMaterial(MMaterial* pMaterial)
{
	if (m_pScene)
		m_pScene->CancelRecordMeshInstance(this);

	if (m_pMaterial)
		m_pMaterial->SubRef();

	if(m_pMaterial = pMaterial)
		m_pMaterial->AddRef();

	if (m_pScene)
		m_pScene->RecordMeshInstance(this);
}

MBoundsAABB* MSkinnedMeshInstance::GetBoundsAABB()
{
	//TODO get the Bounding Box by skeleton instance.

	if (nullptr == m_pBoundsAABB)
		m_pBoundsAABB = new MBoundsAABB();

	if (m_bBoundsAABBDirty)
	{
		Matrix4 matWorldTrans = GetWorldTransform();
		Vector3 v3Position = GetWorldPosition();
		m_pBoundsAABB->SetBoundsOBB(v3Position, matWorldTrans, *m_pMesh->GetMeshesDefaultOBB());
		m_bBoundsAABBDirty = false;
	}

	return m_pBoundsAABB;
}

void MSkinnedMeshInstance::SetMeshData(MModelMeshData* pMeshData)
{
	m_pMesh = pMeshData;
	SetMaterial(pMeshData->GetDefaultMaterial());
}

MIMesh* MSkinnedMeshInstance::GetMesh(const unsigned int& unDetailLevel)
{
	if (unDetailLevel == MMESH_LOD_LEVEL_RANGE)
		return m_pMesh->GetMesh();
	else return m_pMesh->GetLevelMesh(unDetailLevel);
}

MIMesh* MSkinnedMeshInstance::GetMesh()
{
	return m_pMesh->GetMesh();
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
