#include "MMaterial.h"
#include "MShader.h"
#include "MShaderBuffer.h"
#include "MShaderResource.h"
#include "MTextureResource.h"
#include "MMaterialResource.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MFileHelper.h"

#include "MRenderSystem.h"
#include "MResourceSystem.h"

#include "MJson.h"
#include "MVariant.h"


MORTY_CLASS_IMPLEMENT(MMaterial, MResource)

MMaterial::MMaterial()
	: MResource()
	, m_VertexResource(nullptr)
	, m_PixelResource(nullptr)
	, m_pVertexShader(nullptr)
	, m_pPixelShader(nullptr)
	, m_eRasterizerType(MERasterizerType::ECullBack)
	, m_eMaterialType(MEMaterialType::EDefault)
	, m_nVertexShaderIndex(0)
	, m_nPixelShaderIndex(0)
	, m_ShaderMacro()
	, m_unMaterialID(MGlobal::M_INVALID_INDEX)
{
    m_vShaderSets[MGlobal::SHADER_PARAM_SET_MATERIAL] = MShaderParamSet(MGlobal::SHADER_PARAM_SET_MATERIAL);
    m_vShaderSets[MGlobal::MGlobal::SHADER_PARAM_SET_FRAME] = MShaderParamSet(MGlobal::SHADER_PARAM_SET_FRAME);
    m_vShaderSets[MGlobal::MGlobal::SHADER_PARAM_SET_MESH] = MShaderParamSet(MGlobal::SHADER_PARAM_SET_MESH);
    m_vShaderSets[MGlobal::SHADER_PARAM_SET_SKELETON] = MShaderParamSet(MGlobal::SHADER_PARAM_SET_SKELETON);
}

MMaterial::~MMaterial()
{
	ClearParams();
}

std::vector<MShaderConstantParam*>* MMaterial::GetShaderParams()
{
	return &m_vShaderSets[MGlobal::SHADER_PARAM_SET_MATERIAL].m_vParams;
}

std::vector<MShaderSampleParam*>* MMaterial::GetSampleParams()
{
	return &m_vShaderSets[MGlobal::SHADER_PARAM_SET_MATERIAL].m_vSamples;
}

std::vector<MShaderTextureParam*>* MMaterial::GetTextureParams()
{
	return &m_vShaderSets[MGlobal::SHADER_PARAM_SET_MATERIAL].m_vTextures;
}

void MMaterial::SetTexutreParam(const MString& strName, MResource* pResource)
{
	static auto funcResChangedFunction = [this, strName, pResource](const uint32_t& eReloadType) {
		if (MTextureResource* pTexResource = dynamic_cast<MTextureResource*>(pResource))
		{
			for (MShaderTextureParam* pParam : GetMaterialParamSet()->m_vTextures)
			{
				if (pParam->strName == strName)
				{
					if (pParam->pTexture != pTexResource->GetTextureTemplate())
					{
						pParam->pTexture = pTexResource->GetTextureTemplate();
						pParam->SetDirty();
					}
					return true;
				}
			}
		}

		return false;
	};


	for (int i = 0; i < GetMaterialParamSet()->m_vTextures.size(); ++i)
	{
		MShaderRefTextureParam* pParam = static_cast<MShaderRefTextureParam*>(GetMaterialParamSet()->m_vTextures[i]);
		if (strName == pParam->strName)
		{
			if (MTextureResource* pTexResource = dynamic_cast<MTextureResource*>(pResource))
			{
				if (MTexture* pTexture = pTexResource->GetTextureTemplate())
				{
					if (pTexture->GetTextureType() != pParam->eType)
					{
						break;
					}
				}
				
				pParam->m_TextureRef.SetResource(pResource);
				pParam->m_TextureRef.SetResChangedCallback(funcResChangedFunction);

				pParam->pTexture = pTexResource->GetTextureTemplate();
				pParam->SetDirty();
				
			}

			break;
		}
	} 
}

void MMaterial::SetTexutreParam(const uint32_t& unIndex, MResource* pTexResource)
{
	if (unIndex >= GetMaterialParamSet()->m_vTextures.size())
		return;

	SetTexutreParam(GetMaterialParamSet()->m_vTextures[unIndex]->strName, pTexResource);
}

MShaderConstantParam* MMaterial::FindShaderParam(const MString& strName)
{
	for (MShaderConstantParam* pParam : GetMaterialParamSet()->m_vParams)
	{
		if (pParam->strName == strName)
			return pParam;
	}

	return nullptr;
}

void MMaterial::CopyFrom(const MResource* pResource)
{
	Unload();

	const MMaterialResource* pMaterial = dynamic_cast<const MMaterialResource*>(pResource);
	if (nullptr == pMaterial)
		return;

	//Material
	m_VertexResource = pMaterial->m_VertexResource;
	m_PixelResource = pMaterial->m_PixelResource;

	m_pVertexShader = pMaterial->m_pVertexShader;
	m_pPixelShader = pMaterial->m_pPixelShader;

	m_eRasterizerType = pMaterial->m_eRasterizerType;
	m_eMaterialType = pMaterial->m_eMaterialType;

	m_ShaderMacro = pMaterial->m_ShaderMacro;
	
	for (uint32_t i = 0; i < MGlobal::BONES_MAX_NUMBER; ++i)
	{
		CopyShaderParamSet(m_vShaderSets[i], pMaterial->m_vShaderSets[i]);
	}
}

void MMaterial::Encode(MString& strCode)
{
	strCode.clear();

	MStruct material;
	material.AppendMVariant("VS", m_VertexResource.GetResource()->GetResourcePath());
	material.AppendMVariant("PS", m_PixelResource.GetResource()->GetResourcePath());
	material.AppendMVariant("RasterizerType", static_cast<int>(m_eRasterizerType));
	material.AppendMVariant("MaterialType", static_cast<int>(m_eMaterialType));

	MStruct macro;
	m_ShaderMacro.WriteToStruct(macro);
	material.AppendMVariant("macro", macro);

	MStruct vTextures;
	for (MShaderTextureParam* pParam : GetMaterialParamSet()->m_vTextures)
	{
		MShaderRefTextureParam* pRefParam = static_cast<MShaderRefTextureParam*>(pParam);

		if (pRefParam->m_TextureRef.GetResource())
			vTextures.AppendMVariant(pRefParam->strName, pRefParam->m_TextureRef.GetResource()->GetResourcePath());
		else
			vTextures.AppendMVariant(pRefParam->strName, "");
	}

	material.AppendMVariant("textures", vTextures);

	MStruct vParams;
	for (MShaderConstantParam* pParam : GetMaterialParamSet()->m_vParams)
	{
		vParams.AppendMVariant(pParam->strName, pParam->var);
	}

	material.AppendMVariant("params", vParams);


	MVariant variant(material);


	MJson::MVariantToJson(variant, strCode);
}

void MMaterial::Decode(MString& strCode)
{
	MResourceSystem* pSystem = m_pEngine->FindSystem<MResourceSystem>();

	MVariant variant;
	MJson::JsonToMVariant(strCode, variant);

	MStruct& material = *variant.GetStruct();

	MVariant* pVS = material.FindMember("VS");
	MVariant* pPS = material.FindMember("PS");
	MVariant* pRasterizerType = material.FindMember("RasterizerType");
	MVariant* pMaterialType = material.FindMember("MaterialType");
	MVariant* pMacro = material.FindMember("macro");
	MVariant* pTextures = material.FindMember("textures");
	MVariant* pParams = material.FindMember("params");

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
	m_ShaderMacro.ReadFromStruct(macro);

	if (MString* pVSResourcePath = pVS->GetString())
	{
		MResource* pVSRes = pSystem->LoadResource(*pVSResourcePath);
		LoadVertexShader(pVSRes);
	}
	
	if (MString* pPSResourcePath = pPS->GetString())
	{
		MResource* pPsRes = pSystem->LoadResource(*pPSResourcePath);
		LoadPixelShader(pPsRes);
	}


	MStruct& textures = *pTextures->GetStruct();
	for (uint32_t i = 0; i < textures.GetMemberCount(); ++i)
	{
		MStruct::MStructMember* pMember = textures.GetMember(i);
		if (MString* pResourcePath = pMember->var.GetString())
		{
			MResource* pTextureRes = pSystem->LoadResource(*pResourcePath);
			SetTexutreParam(pMember->strName, pTextureRes);
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

void MMaterial::CopyShaderParamSet(MShaderParamSet& target, const MShaderParamSet& source)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	target.DestroyBuffer(pRenderSystem->GetDevice());

	target.m_vParams.resize(source.m_vParams.size());
	for (uint32_t i = 0; i < source.m_vParams.size(); ++i)
	{
		target.m_vParams[i] = new MShaderConstantParam(*source.m_vParams[i], 0);
	}

	target.m_vTextures.resize(source.m_vTextures.size());
	for (uint32_t i = 0; i < source.m_vTextures.size(); ++i)
	{
		MShaderRefTextureParam* pSource = static_cast<MShaderRefTextureParam*>(source.m_vTextures[i]);
		MShaderRefTextureParam* pParam = new MShaderRefTextureParam(*pSource);

		pParam->pTexture = pSource->pTexture;
		pParam->m_TextureRef = pSource->m_TextureRef;

		target.m_vTextures[i] = pParam;
	}

	target.m_vSamples.resize(source.m_vSamples.size());
	for (uint32_t i = 0; i < source.m_vSamples.size(); ++i)
	{
		target.m_vSamples[i] = new MShaderSampleParam(*source.m_vSamples[i]);
	}
}

void MMaterial::SetRasterizerType(const MERasterizerType& eType)
{
	if (m_eRasterizerType == eType)
		return;

	m_eRasterizerType = eType;

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	if (pRenderSystem->GetDevice()->UnRegisterMaterial(this))
	{
		pRenderSystem->GetDevice()->RegisterMaterial(this);
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
		m_ShaderMacro.SetInnerMacro("MEN_TRANSPARENT", "1");
		LoadVertexShader(m_VertexResource.GetResource());
		LoadPixelShader(m_PixelResource.GetResource());
		break;
	}

	default:
		m_ShaderMacro.SetInnerMacro("MEN_TRANSPARENT", "0");
		LoadVertexShader(m_VertexResource.GetResource());
		LoadPixelShader(m_PixelResource.GetResource());
		break;
	}

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	if (pRenderSystem->GetDevice()->UnRegisterMaterial(this))
	{
		pRenderSystem->GetDevice()->RegisterMaterial(this);
	}
}

bool MMaterial::LoadVertexShader(MResource* pResource)
{

	if (MShaderResource* pShaderResource = dynamic_cast<MShaderResource*>(pResource))
	{
		if (MEShaderType::EVertex == pShaderResource->GetShaderType())
		{
			auto LoadFunc = [this](const uint32_t& eReloadType) {
				MShaderResource* pShaderResource = m_VertexResource.GetResource()->DynamicCast<MShaderResource>();
				if (nullptr == pShaderResource)
					return false;

				m_nVertexShaderIndex = pShaderResource->FindShaderByMacroParam(m_ShaderMacro);
				m_pVertexShader = pShaderResource->GetShaderByIndex(m_nVertexShaderIndex);
				if (m_pVertexShader && nullptr == m_pVertexShader->GetBuffer())
				{
					MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
					if (!m_pVertexShader->CompileShader(pRenderSystem->GetDevice()))
					{
						m_pVertexShader = nullptr;
						return false;
					}
				}
				BindShaderBuffer(m_pVertexShader->GetBuffer(), MEShaderParamType::EVertex);
				OnReload();
				return true;
			};


			m_VertexResource.SetResource(pResource);
			m_VertexResource.SetResChangedCallback(LoadFunc);

			LoadFunc(0);
			return true;
		}
	}

	return false;
}

bool MMaterial::LoadVertexShader(const MString& strResource)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (MResource* pResource = pResourceSystem->LoadResource(strResource))
		return LoadVertexShader(pResource);

	return false;
}

bool MMaterial::LoadPixelShader(MResource* pResource)
{
	if (MShaderResource* pShaderResource = dynamic_cast<MShaderResource*>(pResource))
	{
		if (MEShaderType::EPixel == pShaderResource->GetShaderType())
		{
			auto LoadFunc = [this](const uint32_t& eReloadType) {
				MShaderResource* pShaderResource = m_PixelResource.GetResource()->DynamicCast<MShaderResource>();
				if (nullptr == pShaderResource)
					return false;

				m_nPixelShaderIndex = pShaderResource->FindShaderByMacroParam(m_ShaderMacro);
				m_pPixelShader = pShaderResource->GetShaderByIndex(m_nPixelShaderIndex);
				if (m_pPixelShader && nullptr == m_pPixelShader->GetBuffer())
				{
					MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
					if (!m_pPixelShader->CompileShader(pRenderSystem->GetDevice()))
					{
						m_pPixelShader = nullptr;
						return false;
					}
				}

				BindShaderBuffer(m_pPixelShader->GetBuffer(), MEShaderParamType::EPixel);
				OnReload();
				return true;
			};

			m_PixelResource.SetResource(pResource);
			m_PixelResource.SetResChangedCallback(LoadFunc);

			LoadFunc(0);
			return true;
		}
	}

	return false;
}

bool MMaterial::LoadPixelShader(const MString& strResource)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (MResource* pResource = pResourceSystem->LoadResource(strResource))
		return LoadPixelShader(pResource);

	return false;
}

void MMaterial::OnCreated()
{
	Super::OnCreated();

	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	pRenderSystem->GetDevice()->RegisterMaterial(this);
}

void MMaterial::OnDelete()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	if (pRenderSystem->GetDevice())
	{
		pRenderSystem->GetDevice()->UnRegisterMaterial(this);
	}

	m_VertexResource.SetResource(nullptr);
	m_PixelResource.SetResource(nullptr);
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;

	MResource::OnDelete();
}

void MMaterial::Unload()
{
	ClearParams();
	
	m_VertexResource.SetResource(nullptr);
	m_PixelResource.SetResource(nullptr);
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
}

void MMaterial::BindShaderBuffer(MShaderBuffer* pBuffer, const MEShaderParamType& eType)
{
	for (uint32_t i = 0; i < MGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		MShaderParamSet& bufferSet = pBuffer->m_vShaderSets[i];
		MShaderParamSet& selfSet = m_vShaderSets[i];

		for (MShaderConstantParam* pBufferParam : bufferSet.m_vParams)
		{
			if (MShaderConstantParam* pSelfParam = selfSet.FindConstantParam(pBufferParam))
			{
				pSelfParam->eShaderType |= eType;
				MVariant temp;
				temp.Move(pSelfParam->var);
				pSelfParam->var = pBufferParam->var;
				pSelfParam->var.MergeFrom(temp);
				pSelfParam->SetDirty();
			}
			else
			{
				MShaderConstantParam* pParam = new MShaderConstantParam(*pBufferParam, 0);
				pParam->eShaderType = eType;
				selfSet.AppendConstantParam(pParam, eType);
			}
		}

		for (MShaderTextureParam* pBufferParam : bufferSet.m_vTextures)
		{
			if (MShaderTextureParam* pSelfParam = selfSet.FindTextureParam(pBufferParam))
			{
				pSelfParam->eShaderType |= eType;
			}
			else
			{
				MShaderTextureParam* pParam = new MShaderRefTextureParam(*pBufferParam);
				pParam->eShaderType = eType;
				selfSet.AppendTextureParam(pParam, eType);
			}
		}

		for (MShaderSampleParam* pBufferParam : bufferSet.m_vSamples)
		{
			if (MShaderSampleParam* pSelfParam = selfSet.FindSampleParam(pBufferParam))
			{
				pSelfParam->eShaderType |= eType;
			}
			else
			{
				MShaderSampleParam* pParam = new MShaderSampleParam(*pBufferParam);
				pParam->eShaderType = eType;
				selfSet.AppendSampleParam(pParam, eType);
			}
		}
	}
}

void MMaterial::UnbindShaderBuffer(const MEShaderParamType& eType)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	for (uint32_t i = 0; i < MGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		MShaderParamSet& selfSet = m_vShaderSets[i];
		std::vector<MShaderConstantParam*> vConstantParams = selfSet.RemoveConstantParam(eType);
		std::vector<MShaderTextureParam*> vTextureParams = selfSet.RemoveTextureParam(eType);
		std::vector<MShaderSampleParam*> vSampleParams = selfSet.RemoveSampleParam(eType);


		for (MShaderConstantParam* pParam : vConstantParams)
		{
			pRenderSystem->GetDevice()->DestroyShaderParamBuffer(pParam);
			delete pParam;
		}

		for (MShaderTextureParam* pParam : vTextureParams)
			delete pParam;

		for (MShaderSampleParam* pParam : vSampleParams)
			delete pParam;
	}
}

void MMaterial::ClearParams()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	for (uint32_t i = 0; i < MGlobal::SHADER_PARAM_SET_NUM; ++i)
	{
		m_vShaderSets[i].DestroyBuffer(pRenderSystem->GetDevice());
		m_vShaderSets[i] = MShaderParamSet(i);
	}
}

MShaderRefTextureParam::MShaderRefTextureParam()
	: MShaderTextureParam()
	, m_TextureRef()
{

}

MShaderRefTextureParam::MShaderRefTextureParam(const MShaderTextureParam& param)
	: MShaderTextureParam(param)
	, m_TextureRef()
{

}
