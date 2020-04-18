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

MMaterial::MMaterial()
	: MResource()
	, m_pVertexResource(nullptr)
	, m_pPixelResource(nullptr)
	, m_pVertexShader(nullptr)
	, m_pPixelShader(nullptr)
	, m_eRenderState(MIRenderer::ESolid | MIRenderer::ECullBack)
	, m_eBlendState(MIRenderer::ETransparent)
{

}

MMaterial::~MMaterial()
{
	Unload();
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
					MResourceHolder* pNewHolder = new MResourceHolder(pResource);

					if (m_vTextureResHolder[i])
						delete m_vTextureResHolder[i];

					m_vTextureResHolder[i] = pNewHolder;

					m_vTextureResHolder[i]->SetResChangedCallback(funcResChangedFunction);

					param.pTexture = pTexResource->GetTextureTemplate();
				}
			}
			else if (param.eType == ETextureCube)
			{
				if (MTextureCubeResource* pTexResource = dynamic_cast<MTextureCubeResource*>(pResource))
				{
					if (m_vTextureResHolder[i])
						delete m_vTextureResHolder[i];
					m_vTextureResHolder[i] = new MResourceHolder(pResource);
					m_vTextureResHolder[i]->SetResChangedCallback(funcResChangedFunction);

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
			RecompileShaderTextureParam(m_vTextureParams, m_vTextureResHolder, m_pVertexShader->GetBuffer()->m_vTextureParamsTemplate, MEShaderParamType::EVertex);
		}

		if (MEShaderParamType::EVertex != eType)
		{
			RecompileShaderParams(m_vShaderParams, m_pPixelShader->GetBuffer()->m_vShaderParamsTemplate, MEShaderParamType::EPixel);
			RecompileShaderTextureParam(m_vTextureParams, m_vTextureResHolder, m_pPixelShader->GetBuffer()->m_vTextureParamsTemplate, MEShaderParamType::EPixel);
		}
	}
}

void MMaterial::Encode(MString& strCode)
{
	strCode.clear();

	MStruct material;
	material.AppendMVariant("VS", m_pVertexResource->GetResource()->GetResourcePath());
	material.AppendMVariant("PS", m_pPixelResource->GetResource()->GetResourcePath());
	material.AppendMVariant("RenderState", static_cast<int>(m_eRenderState));

	MStruct vTextures;
	for (unsigned int i = 0 ; i < m_vTextureResHolder.size(); ++i)
	{
		MResourceHolder* pTexResource = m_vTextureResHolder[i];
		MShaderTextureParam& param = m_vTextureParams[i];

		if (pTexResource && pTexResource->GetResource())
			vTextures.AppendMVariant(param.strName, pTexResource->GetResource()->GetResourcePath());
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
	MJson::MVariantToJson(variant, strCode);

	MStruct& material = *variant.GetStruct();

	MVariant* pVS = material.FindMember("VS");
	MVariant* pPS = material.FindMember("PS");
	MVariant* pRenderState = material.FindMember("RenderState");
	MVariant* pTextures = material.FindMember("textures");
	MVariant* pParams = material.FindMember("params");

	if (nullptr == pVS) return;
	if (nullptr == pPS) return;
	if (nullptr == pRenderState) return;
	if (nullptr == pTextures) return;
	if (nullptr == pParams) return;

	MResource* pVSRes = pManager->LoadResource(*pVS->GetString());
	MResource* pPsRes = pManager->LoadResource(*pPS->GetString());

	LoadVertexShader(pVSRes);
	LoadPixelShader(pPsRes);

	unsigned int unState = *pRenderState->GetInt();
	SetRenderState(unState);

	MStruct& textures = *pTextures->GetStruct();
	for (unsigned int i = 0; i < textures.GetMemberCount(); ++i)
	{
		MStruct::MStructMember* pMember = textures.GetMember(i);
		MResource* pTextureRes = pManager->LoadResource(*pMember->var.GetString());
		SetTexutreParam(pMember->strName, pTextureRes);
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
			m_pVertexShader = pShaderResource->GetShaderTemplate();
			if (m_pVertexShader && nullptr == m_pVertexShader->GetBuffer())
			{
				if (!m_pVertexShader->CompileShader(m_pEngine->GetDevice()))
				{
					m_pVertexShader = nullptr;
					return false;
				}
			}


			if (m_pVertexResource)
				delete m_pVertexResource;
			m_pVertexResource = new MResourceHolder(pResource);
			m_pVertexResource->SetResChangedCallback([this](const unsigned int& eReloadType) {
				CompileShaderParams(MEShaderParamType::EVertex);
				OnReload();
				return true;
				});

			CompileShaderParams(MEShaderParamType::EVertex);
			OnReload();
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
			m_pPixelShader = pShaderResource->GetShaderTemplate();
			if (m_pPixelShader && nullptr == m_pPixelShader->GetBuffer())
			{
				if (!m_pPixelShader->CompileShader(m_pEngine->GetDevice()))
				{
					m_pPixelShader = nullptr;
					return false;
				}
			}

			if (m_pPixelResource)
				delete m_pPixelResource;
			m_pPixelResource = new MResourceHolder(pResource);
			m_pPixelResource->SetResChangedCallback([this](const unsigned int& eReloadType) {
				CompileShaderParams(MEShaderParamType::EPixel);
				OnReload();
				return true;
				});

			CompileShaderParams(MEShaderParamType::EPixel);
			OnReload();
			return true;
		}
	}

	return false;
}

void MMaterial::Unload()
{
	CleanTextureParams();

	for (MShaderParam& param : m_vShaderParams)
		m_pEngine->GetDevice()->DestroyShaderParamBuffer(&param);

	m_vShaderParams.clear();

	if (m_pVertexResource)
	{
		delete m_pVertexResource;
		m_pVertexResource = nullptr;
	}

	if (m_pPixelResource)
	{
		delete m_pPixelResource;
		m_pPixelResource = nullptr;
	}

	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;

}

const MMaterial& MMaterial::operator=(const MMaterial& mat)
{
	//Material
	m_pVertexResource = new MResourceHolder(*mat.m_pVertexResource);
	m_pPixelResource = new MResourceHolder(*mat.m_pPixelResource);

	m_pVertexShader = mat.m_pVertexShader;
	m_pPixelShader = mat.m_pPixelShader;

	unsigned int m_eRenderState = mat.m_eRenderState;

	m_vShaderParams.resize(mat.m_vShaderParams.size());
	for (unsigned int i = 0; i < mat.m_vShaderParams.size(); ++i)
	{
		m_vShaderParams[i] = mat.m_vShaderParams[i];
	}


	CleanTextureParams();

	m_vTextureParams.resize(mat.m_vTextureParams.size());
	for (unsigned int i = 0; i < mat.m_vTextureParams.size(); ++i)
	{
		m_vTextureParams[i] = mat.m_vTextureParams[i];
	}

	m_vTextureResHolder.resize(mat.m_vTextureResHolder.size(), nullptr);
	for (unsigned int i = 0; i < mat.m_vTextureResHolder.size(); ++i)
	{
		if (mat.m_vTextureResHolder[i])
			m_vTextureResHolder[i] = new MResourceHolder(*mat.m_vTextureResHolder[i]);
	}

	return *this;
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

void MMaterial::RecompileShaderTextureParam(std::vector<MShaderTextureParam>& vParams, std::vector<MResourceHolder*>& vResHolders, std::vector<MShaderTextureParam*>& vNewParams, const MEShaderParamType& eType)
{
	for (unsigned int i = 0; i < vParams.size();)
	{
		MShaderTextureParam& param = vParams[i];
		if (param.eShaderType & eType)
			param.eShaderType = param.eShaderType ^ eType;

		if (0 == param.eType)
		{
			delete vResHolders[i];
			vResHolders[i] = nullptr;

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
			vResHolders.push_back(nullptr);
		}
	}

}

void MMaterial::CleanTextureParams()
{
	m_vTextureParams.clear();

	for (MResourceHolder* pHolder : m_vTextureResHolder)
	{
		if (pHolder)
			delete pHolder;
	}
	m_vTextureResHolder.clear();
}
