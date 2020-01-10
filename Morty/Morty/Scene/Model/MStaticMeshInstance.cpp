#include "MStaticMeshInstance.h"
#include "MModelResource.h"
#include "MScene.h"
#include "MMaterial.h"

#include "MDirectX11Renderer.h"
#include "MModelResource.h"

#include "MBounds.h"

MTypeIdentifierImplement(MStaticMeshInstance, MIMeshInstance)

MStaticMeshInstance::MStaticMeshInstance()
	: MIMeshInstance()
	, m_pMesh(nullptr)
	, m_pMaterial(nullptr)
	, m_pDefaultBoundsOBB(nullptr)
	, m_pBoundsOBB(nullptr)
	, m_bBoundsOBBDirty(true)
{

}

MStaticMeshInstance::~MStaticMeshInstance()
{
	if (m_pBoundsOBB)
	{
		delete m_pBoundsOBB;
		m_pBoundsOBB = nullptr;
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
 
MBoundsOBB* MStaticMeshInstance::GetBoundsOBB()
{
	if (nullptr == m_pBoundsOBB)
		m_pBoundsOBB = new MBoundsOBB();

	if (m_bBoundsOBBDirty)
	{
		Matrix4 matWorldTrans = GetWorldTransform();

		m_pBoundsOBB->m_v3CenterPoint = matWorldTrans * m_pDefaultBoundsOBB->m_v3CenterPoint;
		m_pBoundsOBB->m_matEigVectors = matWorldTrans * m_pDefaultBoundsOBB->m_matEigVectors;
	}

	return m_pBoundsOBB;
}

void MStaticMeshInstance::SetMesh(MIMesh* pMesh)
{
	m_pMesh = dynamic_cast<MMesh<MVertex>*>(pMesh);
}

void MStaticMeshInstance::LocalTransformDirty()
{
	MIMeshInstance::LocalTransformDirty();

	m_bBoundsOBBDirty = true;
}
