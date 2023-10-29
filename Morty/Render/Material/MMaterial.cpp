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

void MMaterial::SetTexture(const MStringId& strName, std::shared_ptr<MResource> pResource)
{
	for (size_t i = 0; i < GetMaterialPropertyBlock()->m_vTextures.size(); ++i)
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

std::shared_ptr<MShaderConstantParam> MMaterial::FindShaderParam(const MStringId& strName)
{
	for (const std::shared_ptr<MShaderConstantParam>& pParam : GetMaterialPropertyBlock()->m_vParams)
	{
		if (pParam->strName == strName)
			return pParam;
	}

	return nullptr;
}

std::shared_ptr<MShaderSampleParam> MMaterial::FindSample(const MStringId& strName)
{
	for (const std::shared_ptr<MShaderSampleParam>& pParam : GetMaterialPropertyBlock()->m_vSamples)
	{
		if (pParam->strName == strName)
			return pParam;
	}

	return nullptr;
}

std::shared_ptr<MShaderTextureParam> MMaterial::FindTexture(const MStringId& strName)
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
		auto shaderMacro = GetShaderMacro();
		shaderMacro.SetInnerMacro(MRenderGlobal::MEN_TRANSPARENT, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
		SetShaderMacro(shaderMacro);
		break;
	}

	default:
		auto shaderMacro = GetShaderMacro();
		shaderMacro.SetInnerMacro(MRenderGlobal::MEN_TRANSPARENT, MRenderGlobal::SHADER_DEFINE_DISABLE_FLAG);
		SetShaderMacro(shaderMacro);
		break;
	}
}

bool MMaterial::LoadShader(std::shared_ptr<MResource> pResource)
{
	bool bResult = m_pShaderProgram->LoadShader(pResource);

	return bResult;
}

bool MMaterial::LoadShader(const MString& strResource)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
		return LoadShader(pResource);

	return false;
}

void MMaterial::SetShaderMacro(const MShaderMacro& macro)
{
	m_pShaderProgram->SetShaderMacro(macro);
}

void MMaterial::OnCreated()
{
	Super::OnCreated();

	m_pShaderProgram = MShaderProgram::MakeShared(GetEngine(), MShaderProgram::EUsage::EGraphics);
}

void MMaterial::OnDelete()
{
	m_pShaderProgram->ClearShader();
	m_pShaderProgram = nullptr;
		
	Super::OnDelete();
}

void MMaterial::Unload()
{
	m_pShaderProgram->ClearShader();
}

#if MORTY_DEBUG
const char* MMaterial::GetDebugName() const
{
	return GetResourcePath().c_str();
}
#endif
