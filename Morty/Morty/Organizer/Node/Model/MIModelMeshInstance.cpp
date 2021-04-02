#include "MIModelMeshInstance.h"
#include "MModelInstance.h"

M_I_OBJECT_IMPLEMENT(MIModelMeshInstance, MIMeshInstance)

MIModelMeshInstance::MIModelMeshInstance()
	: MIMeshInstance()
	, m_eShadowType(MEShadowType::EOnlyDirectional)
	, m_unDetailLevel(MGlobal::MMESH_LOD_LEVEL_RANGE)
	, m_bModelInstanceFound(false)
	, m_pModelInstance(nullptr)
	, m_bDrawBoundingSphere(false)
	, m_bGenerateDirLightShadow(true)
{

}

MIModelMeshInstance::~MIModelMeshInstance()
{

}

MModelInstance* MIModelMeshInstance::GetAttachedModelInstance()
{
	if (!m_bModelInstanceFound && !m_pModelInstance)
	{
		m_pModelInstance = FindParentByType<MModelInstance>();
		m_bModelInstanceFound = true;
	}
	return m_pModelInstance;
}

bool MIModelMeshInstance::GetGenerateDirLightShadow() const
{
	return m_bGenerateDirLightShadow;
}

void MIModelMeshInstance::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);

	M_SERIALIZER_BEGIN(Write);
	M_SERIALIZER_WRITE_VALUE("GenDirShadow", GetGenerateDirLightShadow);
	M_SERIALIZER_WRITE_VALUE("DrawBounding", GetDrawBoundingSphere);
	M_SERIALIZER_WRITE_VALUE("LOD", (int)GetDetailLevel);
	M_SERIALIZER_END;
}

void MIModelMeshInstance::ReadFromStruct(MStruct& srt)
{
	Super::ReadFromStruct(srt);

	M_SERIALIZER_BEGIN(Read);
	M_SERIALIZER_READ_VALUE("GenDirShadow", SetGenerateDirLightShadow, Bool);
	M_SERIALIZER_READ_VALUE("DrawBounding", SetDrawBoundingSphere, Bool);
	M_SERIALIZER_READ_VALUE("LOD", SetDetailLevel, Int);
	M_SERIALIZER_END;
}

void MIModelMeshInstance::ParentChangeImpl(MNode* pParent)
{
	Super::ParentChangeImpl(pParent);

	m_bModelInstanceFound = false;
	m_pModelInstance = nullptr;
}
