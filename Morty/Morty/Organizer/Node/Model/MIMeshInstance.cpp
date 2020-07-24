#include "MIMeshInstance.h"

#include "MScene.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

M_I_OBJECT_IMPLEMENT(MIMeshInstance, M3DNode)

MIMeshInstance::MIMeshInstance()
	: M3DNode()
	, m_pShaderParamSet(nullptr)
	, m_pTransformParam(nullptr)
	, m_bTransformParamDirty(true)
	, m_pWorldMatrixParam(nullptr)
	, m_pNormalMatrixParam(nullptr)
{

}

MIMeshInstance::~MIMeshInstance()
{
	BindShaderParam(nullptr);
}

void MIMeshInstance::BindShaderParam(MMaterial* pMaterial)
{
	if (m_pShaderParamSet)
	{
		m_pShaderParamSet->ClearAndDestroy(GetEngine()->GetDevice());
		delete m_pShaderParamSet;
		m_pShaderParamSet = nullptr;
		m_pTransformParam = nullptr;
	}

	if (pMaterial)
	{
		if (MShaderParamSet* pParamSet = pMaterial->GetMaterialParamSet())
		{
			m_pShaderParamSet = pParamSet->Clone();

			if (m_pTransformParam = pParamSet->FindConstantParam(""))
			{
				if (MStruct* pSrt = m_pTransformParam->var.GetStruct())
				{
					m_pWorldMatrixParam = pSrt->FindMember<Matrix4>("U_matWorld");
					m_pNormalMatrixParam = pSrt->FindMember<Matrix3>("U_matNormal");
				}
			}
		}
	}
}

void MIMeshInstance::SetMaterial(MMaterial* pMaterial)
{
	if (m_Material.GetResource() == pMaterial)
		return;

	if (m_pScene)
		m_pScene->RemoveMaterialGroup(this);

	m_Material.SetResource(pMaterial);
	BindShaderParam(pMaterial);

	if (m_pScene)
		m_pScene->InsertMaterialGroup(this);

}

MMaterial* MIMeshInstance::GetMaterial()
{
	return static_cast<MMaterial*>(m_Material.GetResource());
}

bool MIMeshInstance::SetMaterialPath(const MString& strPath)
{
	if (MResource* pResource = m_pEngine->GetResourceManager()->LoadResource(strPath))
	{
		if (MMaterialResource* pMaterialRes = pResource->DynamicCast<MMaterialResource>())
		{
			SetMaterial(pMaterialRes);
			return true;
		}
	}

	return false;
}

MString MIMeshInstance::GetMaterialPath()
{
	return GetMaterial() ? GetMaterial()->GetResourcePath() : "";
}

void MIMeshInstance::WorldTransformDirty()
{
	Super::WorldTransformDirty();

	m_bTransformParamDirty = true;
}

void MIMeshInstance::LocalTransformDirty()
{
	Super::LocalTransformDirty();

	m_bTransformParamDirty = true;
}

MShaderParamSet* MIMeshInstance::GetShaderMeshParamSet()
{
	if (m_bTransformParamDirty)
	{
		UpdateShaderMeshParam();
	}

	return m_pShaderParamSet;
}

void MIMeshInstance::UpdateShaderMeshParam()
{
	if (m_pTransformParam)
	{
		Matrix4 worldTrans = GetWorldTransform();

		if (m_pWorldMatrixParam)
		{
			*m_pWorldMatrixParam = worldTrans;
		}

		if (m_pNormalMatrixParam)
		{
			//Transposed and Inverse.
			Matrix3 matNormal(worldTrans.Transposed().Inverse(), 3, 3);

			*m_pNormalMatrixParam = matNormal;
		}

		m_pTransformParam->SetDirty();
	}

	m_bTransformParamDirty = false;
}