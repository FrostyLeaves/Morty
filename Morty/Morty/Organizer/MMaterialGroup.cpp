#include "MMaterialGroup.h"
#include "MFunction.h"

MMaterialGroup::MMaterialGroup()
	: m_pMaterial(nullptr)
	, m_vMeshInstances()
	, m_bDirty(true)
{

}

bool MMaterialGroup::InsertMeshInstance(MIMeshInstance* pMeshIns)
{
	m_vMeshInstances.push_back(pMeshIns);
	SetDirty();
	return true;
}

bool MMaterialGroup::RemoveMeshInstance(MIMeshInstance* pMeshIns)
{
	if (ERASE_FIRST_VECTOR(m_vMeshInstances, pMeshIns))
	{
		SetDirty();
		return true;
	}
}
