#include "MIModelMeshInstance.h"
#include "MModelInstance.h"

MTypeIdentifierImplement(MIModelMeshInstance, MIMeshInstance)

MIModelMeshInstance::MIModelMeshInstance()
	: MIMeshInstance()
	, m_eShadowType(MEShadowType::EOnlyDirectional)
	, m_pModelInstance(nullptr)
	, m_unDetailLevel(MMESH_LOD_LEVEL_RANGE)
	, m_bDrawBoundingSphere(false)
	, m_bGenerateDirLightShadow(false)
{

}

MIModelMeshInstance::~MIModelMeshInstance()
{

}

bool MIModelMeshInstance::GetGenerateDirLightShadow() const
{
	return m_bGenerateDirLightShadow && m_pModelInstance->GetGenerateDirLightShadow();
}
