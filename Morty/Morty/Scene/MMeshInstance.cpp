#include "MMeshInstance.h"
#include "MModelResource.h"

#include "MDirectX11Renderer.h"
#include "MModelResource.h"
#include "MModel.h"

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
	m_pMaterial = pMaterial;
}

void MMeshInstance::SetMesh(MIMesh* pMesh)
{
	m_pMesh = pMesh;
}
