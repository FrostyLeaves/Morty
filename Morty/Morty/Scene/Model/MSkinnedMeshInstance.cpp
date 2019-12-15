#include "MSkinnedMeshInstance.h"
#include "MIScene.h"
#include "MMaterial.h"

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

	m_pMaterial = pMaterial;
	m_pMaterial->AddRef();

	if (m_pScene)
		m_pScene->RecordMeshInstance(this);
}

void MSkinnedMeshInstance::SetMesh(MIMesh* pMesh)
{
	m_pMesh = pMesh;
}
