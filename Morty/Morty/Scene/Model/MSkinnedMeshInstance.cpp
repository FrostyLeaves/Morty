#include "MSkinnedMeshInstance.h"
#include "MIScene.h"

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

	m_pMaterial = pMaterial;

	if (m_pScene)
		m_pScene->RecordMeshInstance(this);
}

void MSkinnedMeshInstance::SetMesh(MIMesh* pMesh)
{
	m_pMesh = pMesh;
}
