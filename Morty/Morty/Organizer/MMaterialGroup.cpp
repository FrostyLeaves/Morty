#include "MMaterialGroup.h"
#include "MFunction.h"

MMaterialGroup::MMaterialGroup()
	: m_pMaterial(nullptr)
	, m_vMeshComponents()
	, m_bDirty(true)
{

}

bool MMaterialGroup::InsertMeshComponent(MRenderableMeshComponent* pMeshComponent)
{
	m_vMeshComponents.push_back(pMeshComponent);
	SetDirty();
	return true;
}

bool MMaterialGroup::RemoveMeshComponent(MRenderableMeshComponent* pMeshComponent)
{
	if (ERASE_FIRST_VECTOR(m_vMeshComponents, pMeshComponent))
	{
		SetDirty();
		return true;
	}

	return false;
}

