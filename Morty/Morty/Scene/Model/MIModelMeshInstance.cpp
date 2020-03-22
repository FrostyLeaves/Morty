#include "MIModelMeshInstance.h"

MTypeIdentifierImplement(MIModelMeshInstance, MIMeshInstance)

MIModelMeshInstance::MIModelMeshInstance()
	: MIMeshInstance()
	, m_eShadowType(MEShadowType::EOnlyDirectional)
	, m_pModelInstance(nullptr)
	, m_unDetailLevel(MMESH_LOD_LEVEL_RANGE)
{

}

MIModelMeshInstance::~MIModelMeshInstance()
{

}
