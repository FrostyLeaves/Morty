#include "MStaticMeshInstance.h"
#include "MModelResource.h"
#include "MIScene.h"
#include "MMaterial.h"

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

	if (m_pMaterial)
		m_pMaterial->SubRef();

	if(m_pMaterial = pMaterial)
		m_pMaterial->AddRef();

	if(m_pScene)
		m_pScene->RecordMeshInstance(this);
}
 
void MStaticMeshInstance::SetMesh(MIMesh* pMesh)
{
	m_pMesh = pMesh;
}
