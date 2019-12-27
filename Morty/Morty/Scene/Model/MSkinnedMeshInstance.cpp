#include "MSkinnedMeshInstance.h"
#include "MScene.h"
#include "MMaterial.h"

MTypeIdentifierImplement(MSkinnedMeshInstance, MIMeshInstance)

MSkinnedMeshInstance::MSkinnedMeshInstance()
	: MIMeshInstance()
	, m_pMesh(nullptr)
	, m_pMaterial(nullptr)
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

void MSkinnedMeshInstance::SetMesh(MIMesh* pMesh)
{
	m_pMesh = pMesh;
}
