#include "Material/MMaterial.h"
#include "Material/MShader.h"
#include "MShaderBuffer.h"
#include "Resource/MShaderResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Utility/MFileHelper.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Utility/MJson.h"
#include "Utility/MVariant.h"


MORTY_CLASS_IMPLEMENT(MMaterial, MResource)

MMaterial::MMaterial()
	: MResource()
	, m_pShaderProgram(MShaderProgram::MakeShared(MShaderProgram::EUsage::EGraphics))
	, m_eRasterizerType(MERasterizerType::ECullBack)
	, m_eMaterialType(MEMaterialType::EDefault)
	, m_unMaterialID(MGlobal::M_INVALID_INDEX)
{
}

MMaterial::~MMaterial()
{
}

std::vector<std::shared_ptr<MShaderConstantParam>>& MMaterial::GetShaderParams()
{
	return m_pShaderProgram->GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]->m_vParams;
}

std::vector<std::shared_ptr<MShaderSampleParam>>& MMaterial::GetSampleParams()
{
	return m_pShaderProgram->GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]->m_vSamples;
}

std::vector<std::shared_ptr<MShaderTextureParam>>& MMaterial::GetTextureParams()
{
	return m_pShaderProgram->GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]->m_vTextures;
}

void MMaterial::SetTexutre(const MString& strName, std::shared_ptr<MResource> pResource)
{
	for (int i = 0; i < GetMaterialParamSet()->m_vTextures.size(); ++i)
	{
		const std::shared_ptr<MTextureResourceParam>& pParam = std::dynamic_pointer_cast<MTextureResourceParam>(GetMaterialParamSet()->m_vTextures[i]);
		if (strName == pParam->strName)
		{
			if (std::shared_ptr<MTextureResource> pTexResource = MTypeClass::DynamicCast<MTextureResource>(pResource))
			{
				if (MTexture* pTexture = pTexResource->GetTextureTemplate())
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
	for (const std::shared_ptr<MShaderConstantParam>& pParam : GetMaterialParamSet()->m_vParams)
	{
		if (pParam->strName == strName)
			return pParam;
	}

	return nullptr;
}

void MMaterial::CopyFrom(std::shared_ptr<const MResource> pResource)
{
	Unload();

	std::shared_ptr<const  MMaterialResource> pMaterial = MTypeClass::DynamicCast<const MMaterialResource>(pResource);
	if (nullptr == pMaterial)
		return;

	//Material
	m_eRasterizerType = pMaterial->m_eRasterizerType;
	m_eMaterialType = pMaterial->m_eMaterialType;

	*m_pShaderProgram = *pMaterial->m_pShaderProgram;

	for (uint32_t i = 0; i < MRenderGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		MShaderProgram::CopyShaderParams(GetEngine(), GetShaderParamSets()[i], pMaterial->GetShaderParamSets()[i]);
	}
}

void MMaterial::Encode(MString& strCode)
{
	strCode.clear();

	MStruct material;
	material.SetValue("VS", GetVertexShaderResource()->GetResourcePath());
	material.SetValue("PS", GetPixelShaderResource()->GetResourcePath());
	material.SetValue("RasterizerType", static_cast<int>(m_eRasterizerType));
	material.SetValue("MaterialType", static_cast<int>(m_eMaterialType));

	MStruct macro;
	GetShaderMacro().WriteToStruct(macro);
	material.SetValue("macro", macro);

	MStruct vTextures;
	for (const std::shared_ptr<MShaderTextureParam>& pParam : GetMaterialParamSet()->m_vTextures)
	{
		std::shared_ptr<MTextureResourceParam> pRefParam = std::dynamic_pointer_cast<MTextureResourceParam>(pParam);

		if (auto&& pTextureResource = pRefParam->GetTextureResource())
			vTextures.SetValue(pRefParam->strName, pTextureResource->GetResourcePath());
		else
			vTextures.SetValue(pRefParam->strName, "");
	}

	material.SetValue("textures", vTextures);

	MStruct vParams;
	for (std::shared_ptr<MShaderConstantParam> pParam : GetMaterialParamSet()->m_vParams)
	{
		vParams.SetValue(pParam->strName, pParam->var);
	}

	material.SetValue("params", vParams);


	MVariant variant(material);


	MJson::MVariantToJson(variant, strCode);
}

void MMaterial::Decode(MString& strCode)
{
	MResourceSystem* pSystem = m_pEngine->FindSystem<MResourceSystem>();

	MVariant variant;
	MJson::JsonToMVariant(strCode, variant);

	MStruct& material = *variant.GetStruct();

	MVariant* pVS = material.GetValue("VS");
	MVariant* pPS = material.GetValue("PS");
	MVariant* pRasterizerType = material.GetValue("RasterizerType");
	MVariant* pMaterialType = material.GetValue("MaterialType");
	MVariant* pMacro = material.GetValue("macro");
	MVariant* pTextures = material.GetValue("textures");
	MVariant* pParams = material.GetValue("params");

	if (nullptr == pVS) return;
	if (nullptr == pPS) return;
	if (nullptr == pRasterizerType) return;
	if (nullptr == pMaterialType) return;
	if (nullptr == pMacro) return;
	if (nullptr == pTextures) return;
	if (nullptr == pParams) return;

	if(int* pType = pRasterizerType->GetInt())
		SetRasterizerType(MERasterizerType(*pType));
	
	if(int* pType = pMaterialType->GetInt())
		SetMaterialType(MEMaterialType(*pType));

	MStruct& macro = *pMacro->GetStruct();
	GetShaderMacro().ReadFromStruct(macro);

	if (MString* pVSResourcePath = pVS->GetString())
	{
		std::shared_ptr<MResource> pVSRes = pSystem->LoadResource(*pVSResourcePath);
		LoadVertexShader(pVSRes);
	}
	
	if (MString* pPSResourcePath = pPS->GetString())
	{
		std::shared_ptr<MResource> pPsRes = pSystem->LoadResource(*pPSResourcePath);
		LoadPixelShader(pPsRes);
	}


	MStruct& textures = *pTextures->GetStruct();
	for (uint32_t i = 0; i < textures.GetMemberCount(); ++i)
	{
		MStruct::MStructMember* pMember = textures.GetMember(i);
		if (MString* pResourcePath = pMember->var.GetString())
		{
			std::shared_ptr<MResource> pTextureRes = pSystem->LoadResource(*pResourcePath);
			SetTexutre(pMember->strName, pTextureRes);
		}
	}

	auto& vShaderParams = GetMaterialParamSet()->m_vParams;

	MStruct& params = *pParams->GetStruct();
	for (uint32_t i = 0; i < params.GetMemberCount(); ++i)
	{
		MStruct::MStructMember* pMember = params.GetMember(i);
		for (uint32_t j = 0; j < vShaderParams.size(); ++j)
		{
			if (vShaderParams[j]->strName == pMember->strName)
			{
				vShaderParams[i]->var.MergeFrom(pMember->var);
				vShaderParams[i]->SetDirty();
				break;
			}
		}
	}

}

bool MMaterial::SaveTo(const MString& strResourcePath)
{
	MString strCode;
	Encode(strCode);

	return MFileHelper::WriteString(strResourcePath, strCode);
}

bool MMaterial::Load(const MString& strResourcePath)
{
	MString strCode;
	if (!MFileHelper::ReadString(strResourcePath, strCode))
		return false;

	Decode(strCode);

	return true;
}

void MMaterial::SetRasterizerType(const MERasterizerType& eType)
{
	if (m_eRasterizerType == eType)
		return;

	m_eRasterizerType = eType;

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	std::shared_ptr<MMaterial> self = MTypeClass::DynamicCast<MMaterial>(GetShared());

	if (pRenderSystem->GetDevice()->UnRegisterMaterial(self))
	{
		pRenderSystem->GetDevice()->RegisterMaterial(self);
	}
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

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	std::shared_ptr<MMaterial> self = MTypeClass::DynamicCast<MMaterial>(GetShared());

	if (pRenderSystem->GetDevice()->UnRegisterMaterial(self))
	{
		pRenderSystem->GetDevice()->RegisterMaterial(self);
	}
}

bool MMaterial::LoadVertexShader(std::shared_ptr<MResource> pResource)
{
	bool bResult = m_pShaderProgram->LoadVertexShader(GetEngine(), pResource);

	OnReload();

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

	OnReload();

	return bResult;
}

bool MMaterial::LoadPixelShader(const MString& strResource)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
		return LoadPixelShader(pResource);

	return false;
}

bool MMaterial::LoadComputeShader(std::shared_ptr<MResource> pResource)
{
	bool bResult = m_pShaderProgram->LoadComputeShader(GetEngine(), pResource);

	OnReload();

	return bResult;
}

bool MMaterial::LoadComputeShader(const MString& strResource)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
		return LoadComputeShader(pResource);

	return false;
}

void MMaterial::OnCreated()
{
	Super::OnCreated();

	std::shared_ptr<MMaterial> self = MTypeClass::DynamicCast<MMaterial>(GetShared());

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	pRenderSystem->GetDevice()->RegisterMaterial(self);
}

void MMaterial::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	if (pRenderSystem->GetDevice())
	{
		std::shared_ptr<MMaterial> self = MTypeClass::DynamicCast<MMaterial>(GetShared());
		pRenderSystem->GetDevice()->UnRegisterMaterial(self);
	}

	m_pShaderProgram->ClearShader(GetEngine());
		
	MResource::OnDelete();
}

void MMaterial::Unload()
{
	m_pShaderProgram->ClearShader(GetEngine());
}

const MString MaterialKey::Albedo = "U_mat_texAlbedo";
const MString MaterialKey::Normal = "U_mat_texNormal";
const MString MaterialKey::Metallic = "U_mat_texMetallic";
const MString MaterialKey::Roughness = "U_mat_texRoughness";
const MString MaterialKey::AmbientOcc = "U_mat_texAmbientOcc";
const MString MaterialKey::Height = "U_mat_texHeight";
