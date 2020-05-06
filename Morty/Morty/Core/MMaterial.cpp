#include "MMaterial.h"
#include "MShader.h"
#include "Shader/MShaderResource.h"
#include "Material/MMaterialResource.h"
#include "Texture/MTextureResource.h"
#include "Texture/MTextureCubeResource.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MIRenderer.h"
#include "MFileHelper.h"
#include "MResourceManager.h"

#include "MVariant.h"
#include "Json/MJson.h"

#include <algorithm>

MTypeIdentifierImplement(MMaterial, MResource)

MMaterial::MMaterial()
	: MResource()
	, m_VertexResource(nullptr)
	, m_PixelResource(nullptr)
	, m_pVertexShader(nullptr)
	, m_pPixelShader(nullptr)
	, m_eRasterizerType(MERasterizerType::ECullBack)
	, m_eMaterialType(MEMaterialType::EDefault)
{
	m_unResourceType = MResourceManager::MEResourceType::Material;
}

MMaterial::~MMaterial()
{
	CleanTextureParams();
	CleanShaderParams();
}

void MMaterial::SetTexutreParam(const MString& strName, MResource* pResource)
{
	static auto funcResChangedFunction = [this, strName, pResource](const unsigned int& eReloadType) {
		if (MTextureResource* pTexResource = dynamic_cast<MTextureResource*>(pResource))
		{
			for (MShaderTextureParam& param : m_vTextureParams)
			{
				if (param.strName == strName)
				{
					param.pTexture = pTexResource->GetTextureTemplate();
					return true;
				}
			}
		}

		return false;
	};


	for (int i = 0; i < m_vTextureParams.size(); ++i)
	{
		MShaderTextureParam& param = m_vTextureParams[i];
		if (strName == param.strName)
		{
			if (param.eType == ETexture2D)
			{
				if (MTextureResource* pTexResource = dynamic_cast<MTextureResource*>(pResource))
				{
					m_vTextureResKeeper[i].SetResource(pResource);
					m_vTextureResKeeper[i].SetResChangedCallback(funcResChangedFunction);

					param.pTexture = pTexResource->GetTextureTemplate();
				}
			}
			else if (param.eType == ETextureCube)
			{
				if (MTextureCubeResource* pTexResource = dynamic_cast<MTextureCubeResource*>(pResource))
				{
					m_vTextureResKeeper[i].SetResource(pTexResource);
					m_vTextureResKeeper[i].SetResChangedCallback(funcResChangedFunction);

					param.pTexture = pTexResource->GetTextureCubeTemplate();
				}
			}

			break;
		}
	} 
}

void MMaterial::SetTexutreParam(const unsigned int& unIndex, MResource* pTexResource)
{
	if (unIndex >= m_vTextureParams.size())
		return;

	SetTexutreParam(m_vTextureParams[unIndex].strName, pTexResource);
}

void MMaterial::CompileShaderParams(const MEShaderParamType& eType)
{
	if (m_pVertexShader && m_pVertexShader->GetBuffer())
	{
		if (MEShaderParamType::EPixel != eType)
		{
			RecompileShaderParams(m_vShaderParams, m_pVertexShader->GetBuffer()->m_vShaderParamsTemplate, MEShaderParamType::EVertex);
			RecompileShaderTextureParam(m_vTextureParams, m_vTextureResKeeper, m_pVertexShader->GetBuffer()->m_vTextureParamsTemplate, MEShaderParamType::EVertex);
		}

		if (MEShaderParamType::EVertex != eType)
		{
			RecompileShaderParams(m_vShaderParams, m_pPixelShader->GetBuffer()->m_vShaderParamsTemplate, MEShaderParamType::EPixel);
			RecompileShaderTextureParam(m_vTextureParams, m_vTextureResKeeper, m_pPixelShader->GetBuffer()->m_vTextureParamsTemplate, MEShaderParamType::EPixel);
		}
	}
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

	m_vShaderParams.resize(pMaterial->m_vShaderParams.size());
	for (unsigned int i = 0; i < pMaterial->m_vShaderParams.size(); ++i)
	{
		m_vShaderParams[i] = pMaterial->m_vShaderParams[i];
		m_vShaderParams[i].pBuffer = nullptr;

		m_pEngine->GetDevice()->GenerateShaderParamBuffer(&m_vShaderParams[i]);
	}

	m_vTextureParams.resize(pMaterial->m_vTextureParams.size());
	for (unsigned int i = 0; i < pMaterial->m_vTextureParams.size(); ++i)
	{
		m_vTextureParams[i] = pMaterial->m_vTextureParams[i];
	}

	m_vTextureResKeeper.resize(pMaterial->m_vTextureResKeeper.size());
	for (unsigned int i = 0; i < pMaterial->m_vTextureResKeeper.size(); ++i)
	{
		m_vTextureResKeeper[i] = pMaterial->m_vTextureResKeeper[i];
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

	MStruct vTextures;
	for (unsigned int i = 0 ; i < m_vTextureResKeeper.size(); ++i)
	{
		MResourceKeeper& texResource = m_vTextureResKeeper[i];
		MShaderTextureParam& param = m_vTextureParams[i];

		if (texResource.GetResource())
			vTextures.AppendMVariant(param.strName, texResource.GetResource()->GetResourcePath());
		else
			vTextures.AppendMVariant(param.strName, "");
	}

	material.AppendMVariant("textures", vTextures);

	MStruct vParams;
	for (MShaderParam param : m_vShaderParams)
	{
		vParams.AppendMVariant(param.strName, param.var);
	}

	material.AppendMVariant("params", vParams);

	MVariant variant(material);


	MJson::MVariantToJson(variant, strCode);
}

void MMaterial::Decode(MString& strCode)
{
	MResourceManager* pManager = m_pEngine->GetResourceManager();

	MVariant variant;
	MJson::JsonToMVariant(strCode, variant);

	MStruct& material = *variant.GetStruct();

	MVariant* pVS = material.FindMember("VS");
	MVariant* pPS = material.FindMember("PS");
	MVariant* pRasterizerType = material.FindMember("RasterizerType");
	MVariant* pMaterialType = material.FindMember("MaterialType");
	MVariant* pTextures = material.FindMember("textures");
	MVariant* pParams = material.FindMember("params");

	if (nullptr == pVS) return;
	if (nullptr == pPS) return;
	if (nullptr == pRasterizerType) return;
	if (nullptr == pMaterialType) return;
	if (nullptr == pTextures) return;
	if (nullptr == pParams) return;

	MResource* pVSRes = pManager->LoadResource(*pVS->GetString());
	MResource* pPsRes = pManager->LoadResource(*pPS->GetString());

	LoadVertexShader(pVSRes);
	LoadPixelShader(pPsRes);

	int unType = *pRasterizerType->GetInt();
	SetRasterizerType(MERasterizerType(unType));
	unType = *pMaterialType->GetInt();
	SetMaterialType((MEMaterialType)unType);


	MStruct& textures = *pTextures->GetStruct();
	for (unsigned int i = 0; i < textures.GetMemberCount(); ++i)
	{
		MStruct::MStructMember* pMember = textures.GetMember(i);
		if (pMember->var.GetType() == MVariant::EString)
		{
			MResource* pTextureRes = pManager->LoadResource(*pMember->var.GetString());
			SetTexutreParam(pMember->strName, pTextureRes);
		}
	}

	MStruct& params = *pParams->GetStruct();
	for (unsigned int i = 0; i < params.GetMemberCount(); ++i)
	{
		MStruct::MStructMember* pMember = params.GetMember(i);
		for (unsigned int j = 0; j < m_vShaderParams.size(); ++j)
		{
			if (m_vShaderParams[j].strName == pMember->strName)
			{
				m_vShaderParams[i].var.MergeFrom(pMember->var);
				m_vShaderParams[i].SetDirty();
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

bool MMaterial::LoadVertexShader(MResource* pResource)
{
	if (MShaderResource* pShaderResource = dynamic_cast<MShaderResource*>(pResource))
	{
		if (MShader::MEShaderType::Vertex == pShaderResource->GetShaderTemplate()->GetType())
		{
			auto LoadFunc = [this](const unsigned int& eReloadType) {

				m_pVertexShader = dynamic_cast<MShaderResource*>(m_VertexResource.GetResource())->GetShaderTemplate();
				if (m_pVertexShader && nullptr == m_pVertexShader->GetBuffer())
				{
					if (!m_pVertexShader->CompileShader(m_pEngine->GetDevice()))
					{
						m_pVertexShader = nullptr;
						return false;
					}
				}

				CompileShaderParams(MEShaderParamType::EVertex);
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

bool MMaterial::LoadPixelShader(MResource* pResource)
{
	if (MShaderResource* pShaderResource = dynamic_cast<MShaderResource*>(pResource))
	{
		if (MShader::MEShaderType::Pixel == pShaderResource->GetShaderTemplate()->GetType())
		{
			auto LoadFunc = [this](const unsigned int& eReloadType) {
				m_pPixelShader = dynamic_cast<MShaderResource*>(m_PixelResource.GetResource())->GetShaderTemplate();
				if (m_pPixelShader && nullptr == m_pPixelShader->GetBuffer())
				{
					if (!m_pPixelShader->CompileShader(m_pEngine->GetDevice()))
					{
						m_pPixelShader = nullptr;
						return false;
					}
				}
				CompileShaderParams(MEShaderParamType::EPixel);
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

void MMaterial::OnDelete()
{
	m_VertexResource.SetResource(nullptr);
	m_PixelResource.SetResource(nullptr);
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;

	MResource::OnDelete();
}

void MMaterial::Unload()
{
	CleanTextureParams();
	CleanShaderParams();
	
	m_VertexResource.SetResource(nullptr);
	m_PixelResource.SetResource(nullptr);
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;
}

void MMaterial::RecompileShaderParams(std::vector<MShaderParam>& vParams, std::vector<MShaderParam*>& vNewParams, const MEShaderParamType& eType)
{
	for (std::vector<MShaderParam>::iterator iter = vParams.begin(); iter != vParams.end();)
	{
		MShaderParam& param = *iter;
		if (param.eType & eType)
			param.eType = param.eType ^ eType;
		
		if (0 == param.eType)
		{
			m_pEngine->GetDevice()->DestroyShaderParamBuffer(&param);
			iter = vParams.erase(iter);
		}
		else
			++iter;
	}

	unsigned int vParamsSize = vParams.size();
	for (unsigned int j = 0; j < vNewParams.size(); ++j)
	{
		bool bFinded = false;
		for (unsigned int i = 0; i < vParamsSize; ++i)
		{
			if (vParams[i].strName == vNewParams[j]->strName)
			{
				vParams[i].eType |= eType;
				MVariant var = vParams[i].var;
				vParams[i].var = vNewParams[j]->var;
				vParams[i].var.MergeFrom(var);
				vParams[i].SetDirty();

				bFinded = true;
				break;
			}
		}

		if (!bFinded)
		{
			MShaderParam param;
			param = *vNewParams[j];
			param.pBuffer = nullptr;
			m_pEngine->GetDevice()->GenerateShaderParamBuffer(&param);
			vParams.push_back(param);
		}
	}

}

void MMaterial::RecompileShaderTextureParam(std::vector<MShaderTextureParam>& vParams, std::vector<MResourceKeeper>& vResHolders, std::vector<MShaderTextureParam*>& vNewParams, const MEShaderParamType& eType)
{
	for (unsigned int i = 0; i < vParams.size();)
	{
		MShaderTextureParam& param = vParams[i];
		if (param.eShaderType & eType)
			param.eShaderType = param.eShaderType ^ eType;

		if (0 == param.eType)
		{
			vResHolders[i].SetResource(nullptr);
		
			vParams.erase(vParams.begin() + i);
			vResHolders.erase(vResHolders.begin() + i);
		}
		else ++i;
	}

	for (unsigned int j = 0; j < vNewParams.size(); ++j)
	{
		bool bFinded = false;
		for (unsigned int i = 0; i < vParams.size(); ++i)
		{
			if (vParams[i].strName == vNewParams[j]->strName)
			{
				vParams[i].eShaderType |= eType;
				bFinded = true;
				break;
			}
		}

		if (!bFinded)
		{
			MShaderTextureParam param;
			param = *vNewParams[j];
			param.eShaderType = eType;
			vParams.push_back(param);
			vResHolders.push_back(MResourceKeeper());
		}
	}

}

void MMaterial::CleanTextureParams()
{
	m_vTextureParams.clear();

	for (MResourceKeeper& keeper : m_vTextureResKeeper)
	{
		keeper.SetResource(nullptr);
	}
	m_vTextureResKeeper.clear();
}

void MMaterial::CleanShaderParams()
{
	for (MShaderParam& param : m_vShaderParams)
		m_pEngine->GetDevice()->DestroyShaderParamBuffer(&param);

	m_vShaderParams.clear();
}
