#include "MMeshInstance.h"
#include "MModelResource.h"
#include "MIScene.h"

#include "MDirectX11Renderer.h"
#include "MModelResource.h"

MMeshInstance::MMeshInstance()
	: M3DNode()
	, m_pMesh(nullptr)
	, m_pMaterial(nullptr)
{

}

MMeshInstance::~MMeshInstance()
{

}

void MMeshInstance::OnTick(const float& fDelta)
{

}

void MMeshInstance::SetMaterial(MMaterial* pMaterial)
{
	if (m_pScene)
		m_pScene->CancelRecordMeshInstance(this);

	m_pMaterial = pMaterial;

	if(m_pScene)
		m_pScene->RecordMeshInstance(this);
}

void MMeshInstance::SetMesh(MIMesh* pMesh)
{
	m_pMesh = pMesh;
}
