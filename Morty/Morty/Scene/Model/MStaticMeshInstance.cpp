#include "MStaticMeshInstance.h"
#include "MModelResource.h"
#include "MScene.h"
#include "MMaterial.h"

#include "MDirectX11Renderer.h"
#include "MModelResource.h"
#include "MModelDetailLevel.h"

#include "MBounds.h"

MTypeIdentifierImplement(MStaticMeshInstance, MIMeshInstance)

MStaticMeshInstance::MStaticMeshInstance()
	: MIMeshInstance()
	, m_pMesh(nullptr)
	, m_pMaterial(nullptr)
	, m_pDefaultBoundsOBB(nullptr)
	, m_pBoundsAABB(nullptr)
	, m_bBoundsAABBDirty(true)
	, m_unDetailLevel(5)
{

}

MStaticMeshInstance::~MStaticMeshInstance()
{
	if (m_pBoundsAABB)
	{
		delete m_pBoundsAABB;
		m_pBoundsAABB = nullptr;
	}
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
	if (nullptr == m_pBoundsAABB)
		m_pBoundsAABB = new MBoundsAABB();

	if (m_bBoundsAABBDirty)
	{
		Matrix4 matWorldTrans = GetWorldTransform();
		Vector3 v3Position = GetWorldPosition();
		m_pBoundsAABB->SetBoundsOBB(v3Position, matWorldTrans, *m_pDefaultBoundsOBB);
		m_bBoundsAABBDirty = false;
	}

	return m_pBoundsAABB;
}

void MStaticMeshInstance::SetMesh(MIMesh* pMesh)
{
	m_pMesh = dynamic_cast<MMesh<MVertex>*>(pMesh);
	if (m_pMesh)
	{
		m_pMeshDetailMap = new MMeshDetailMap(m_pMesh);
	}
}

MIMesh* MStaticMeshInstance::GetMesh(const unsigned int& unDetailLevel)
{
	if (nullptr == m_pMeshDetailMap || unDetailLevel == 10)
		return GetMesh();

	m_pMeshDetailMap->GetLevel(unDetailLevel);
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
}
