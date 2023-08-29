#include "Material/MMaterial.h"
#include "Material/MShader.h"
#include "MShaderBuffer.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"
#include "Engine/MEngine.h"
#include "Flatbuffer/MMaterial_generated.h"
#include "Render/MIDevice.h"
#include "Utility/MFileHelper.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Variant/MVariant.h"


MORTY_CLASS_IMPLEMENT(MMaterial, MResource)

MMaterial::MMaterial()
	: MResource()
	, m_pShaderProgram(MShaderProgram::MakeShared(MShaderProgram::EUsage::EGraphics))
	, m_eCullMode(MECullMode::ECullBack)
	, m_eMaterialType(MEMaterialType::EDefault)
{
}

std::vector<std::shared_ptr<MShaderConstantParam>>& MMaterial::GetShaderParams()
{
	return m_pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]->m_vParams;
}

std::vector<std::shared_ptr<MShaderSampleParam>>& MMaterial::GetSampleParams()
{
	return m_pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]->m_vSamples;
}

std::vector<std::shared_ptr<MShaderTextureParam>>& MMaterial::GetTextureParams()
{
	return m_pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]->m_vTextures;
}

void MMaterial::SetTexture(const MString& strName, std::shared_ptr<MResource> pResource)
{
	for (int i = 0; i < GetMaterialPropertyBlock()->m_vTextures.size(); ++i)
	{
		const std::shared_ptr<MTextureResourceParam>& pParam = std::dynamic_pointer_cast<MTextureResourceParam>(GetMaterialPropertyBlock()->m_vTextures[i]);
		if (strName == pParam->strName)
		{
			if (std::shared_ptr<MTextureResource> pTexResource = MTypeClass::DynamicCast<MTextureResource>(pResource))
			{
				if (std::shared_ptr<MTexture> pTexture = pTexResource->GetTextureTemplate())
				{
					if (pTexture->GetTextureType() != pParam->eType)
					{
						break;
					}
				}
				
				pParam->SetTexture(pTexResource);
			}

			break;
		}
	} 
}

std::shared_ptr<MShaderConstantParam> MMaterial::FindShaderParam(const MString& strName)
{
	for (const std::shared_ptr<MShaderConstantParam>& pParam : GetMaterialPropertyBlock()->m_vParams)
	{
		if (pParam->strName == strName)
			return pParam;
	}

	return nullptr;
}

std::shared_ptr<MShaderSampleParam> MMaterial::FindSample(const MString& strName)
{
	for (const std::shared_ptr<MShaderSampleParam>& pParam : GetMaterialPropertyBlock()->m_vSamples)
	{
		if (pParam->strName == strName)
			return pParam;
	}

	return nullptr;
}

std::shared_ptr<MShaderTextureParam> MMaterial::FindTexture(const MString& strName)
{
	for (const std::shared_ptr<MShaderTextureParam>& pParam : GetMaterialPropertyBlock()->m_vTextures)
	{
		if (pParam->strName == strName)
			return pParam;
	}

	return nullptr;
}

void MMaterial::SetCullMode(const MECullMode& eType)
{
	if (m_eCullMode == eType)
		return;

	m_eCullMode = eType;
}

void MMaterial::SetMaterialType(const MEMaterialType& eType)
{
	if (m_eMaterialType == eType)
		return;

	m_eMaterialType = eType;

	switch (m_eMaterialType)
	{
	case MEMaterialType::EDepthPeel:
	{
		GetShaderMacro().SetInnerMacro("MEN_TRANSPARENT", "1");
		LoadVertexShader(GetVertexShaderResource());
		LoadPixelShader(GetPixelShaderResource());
		break;
	}

	default:
		GetShaderMacro().SetInnerMacro("MEN_TRANSPARENT", "0");
		LoadVertexShader(GetVertexShaderResource());
		LoadPixelShader(GetPixelShaderResource());
		break;
	}
}

bool MMaterial::LoadVertexShader(std::shared_ptr<MResource> pResource)
{
	bool bResult = m_pShaderProgram->LoadVertexShader(GetEngine(), pResource);

	return bResult;
}

bool MMaterial::LoadVertexShader(const MString& strResource)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
		return LoadVertexShader(pResource);

	return false;
}

bool MMaterial::LoadPixelShader(std::shared_ptr<MResource> pResource)
{
	bool bResult = m_pShaderProgram->LoadPixelShader(GetEngine(), pResource);

	return bResult;
}

bool MMaterial::LoadPixelShader(const MString& strResource)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
		return LoadPixelShader(pResource);

	return false;
}

void MMaterial::SetShaderMacro(const MShaderMacro& macro)
{
	m_pShaderProgram->SetShaderMacro(macro);
}

void MMaterial::OnCreated()
{
	Super::OnCreated();
}

void MMaterial::OnDelete()
{
	m_pShaderProgram->ClearShader(GetEngine());
		
	Super::OnDelete();
}

void MMaterial::Unload()
{
	m_pShaderProgram->ClearShader(GetEngine());
}

const MString MaterialKey::Albedo = "u_mat_texAlbedo";
const MString MaterialKey::Normal = "u_texNormal";
const MString MaterialKey::Metallic = "u_mat_texMetallic";
const MString MaterialKey::Roughness = "u_mat_texRoughness";
const MString MaterialKey::AmbientOcc = "u_mat_texAmbientOcc";
const MString MaterialKey::Height = "u_mat_texHeight";
const MString MaterialKey::Emission = "u_mat_texEmission";
