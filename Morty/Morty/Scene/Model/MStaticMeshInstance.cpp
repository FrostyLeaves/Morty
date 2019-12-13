#include "MStaticMeshInstance.h"
#include "MModelResource.h"
#include "MIScene.h"

#include "MDirectX11Renderer.h"
#include "MModelResource.h"

MStaticMeshInstance::MStaticMeshInstance()
	: MIMeshInstance()
	, m_pMesh(nullptr)
	, m_pMaterial(nullptr)
{

}

MStaticMeshInstance::~MStaticMeshInstance()
{

}

void MStaticMeshInstance::SetMaterial(MMaterial* pMaterial)
{
	if (m_pScene)
		m_pScene->CancelRecordMeshInstance(this);

	m_pMaterial = pMaterial;

	if(m_pScene)
		m_pScene->RecordMeshInstance(this);
}
 
void MStaticMeshInstance::SetMesh(MIMesh* pMesh)
{
	m_pMesh = pMesh;
}
