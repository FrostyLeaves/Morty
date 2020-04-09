#include "MMaterial.h"
#include "MShader.h"
#include "Shader/MShaderResource.h"
#include "Material/MMaterialResource.h"
#include "Texture/MTextureResource.h"
#include "Texture/MTextureCubeResource.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MIRenderer.h"

#include "MVariant.h"

#include <algorithm>

MMaterial::MMaterial()
	: MResource()
	, m_pVertexResource(nullptr)
	, m_pPixelResource(nullptr)
	, m_pVertexShader(nullptr)
	, m_pPixelShader(nullptr)
	, m_eRenderState(MIRenderer::ESolid | MIRenderer::ECullBack)
{

}

MMaterial::~MMaterial()
{
	Unload();
}

void MMaterial::SetTexutreParam(const MString& strName, MResource* pResource)
{
	for (int i = 0; i < m_vTextureParams.size(); ++i)
	{
		MShaderTextureParam* pParam = m_vTextureParams[i];
		if (strName == pParam->strName)
		{
			if (pParam->eType == ETexture2D)
			{
				if (MTextureResource* pTexResource = dynamic_cast<MTextureResource*>(pResource))
				{
					MResourceHolder* pNewHolder = new MResourceHolder(pResource);

					if (m_vTextureResHolder[i])
						delete m_vTextureResHolder[i];

					m_vTextureResHolder[i] = pNewHolder;
					m_vTextureResHolder[i]->SetResChangedCallback([pParam, &pTexResource](const unsigned int& eReloadType) {
						pParam->pTexture = pTexResource->GetTextureTemplate();
						return true;
						});

					pParam->pTexture = pTexResource->GetTextureTemplate();
				}
			}
			else if (pParam->eType == ETextureCube)
			{
				if (MTextureCubeResource* pTexResource = dynamic_cast<MTextureCubeResource*>(pResource))
				{
					if (m_vTextureResHolder[i])
						delete m_vTextureResHolder[i];
					m_vTextureResHolder[i] = new MResourceHolder(pResource);
					m_vTextureResHolder[i]->SetResChangedCallback([pParam, &pTexResource](const unsigned int& eReloadType) {
						pParam->pTexture = pTexResource->GetTextureCubeTemplate();
						return true;
						});

					pParam->pTexture = pTexResource->GetTextureCubeTemplate();
				}
			}

			break;
		}
	} 
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
	material.SetMember("VS", m_pVertexResource->GetResource()->GetResourcePath());
	material.SetMember("PS", m_pPixelResource->GetResource()->GetResourcePath());
	material.SetMember("RenderState", static_cast<int>(m_eRenderState));

	MVariantArray vTextures;
	for (MResourceHolder* pTexResource : m_vTextureResHolder)
	{
		if (pTexResource && pTexResource->GetResource())
			vTextures.AppendMVariant(pTexResource->GetResource()->GetResourcePath());
		else
			vTextures.AppendMVariant("");
	}

	material.SetMember("textures", vTextures);

	MVariantArray vParams;
	for (MShaderParam* param : m_vShaderParams)
		vParams.AppendMVariant(param->var);

	material.SetMember("params", vParams);

	MVariant variant(material);
}

void MMaterial::Decode(MString& strCode)
{

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

	for (MShaderParam* pParam : m_vShaderParams)
	{
		m_pEngine->GetDevice()->DestroyShaderParamBuffer(pParam);
		delete pParam;
	}

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
	MResourceHolder* m_pVertexResource = new MResourceHolder(*mat.m_pVertexResource);
	MResourceHolder* m_pPixelResource = new MResourceHolder(*mat.m_pPixelResource);

	MShader* m_pVertexShader = mat.m_pVertexShader;
	MShader* m_pPixelShader = mat.m_pPixelShader;

	unsigned int m_eRenderState = mat.m_eRenderState;

	CompileShaderParams(MEShaderParamType::EBoth);

	return *this;
}

bool MMaterial::Load(const MString& strResourcePath)
{
	return false;
}

void MMaterial::RecompileShaderParams(std::vector<MShaderParam*>& vParams, std::vector<MShaderParam*>& vNewParams, const MEShaderParamType& eType)
{
	for (std::vector<MShaderParam*>::iterator iter = vParams.begin(); iter != vParams.end();)
	{
		MShaderParam* pParam = *iter;
		if (pParam->eType & eType)
			pParam->eType = pParam->eType ^ eType;
		
		if (0 == pParam->eType)
		{
			m_pEngine->GetDevice()->DestroyShaderParamBuffer(pParam);
			delete pParam;
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
			if (vParams[i]->strName == vNewParams[j]->strName)
			{
				vParams[i]->eType |= eType;
				MVariant var = vParams[i]->var;
				vParams[i]->var = vNewParams[j]->var;
				vParams[i]->var.MergeFrom(var);
				vParams[i]->SetDirty();

				bFinded = true;
				break;
			}
		}

		if (!bFinded)
		{
			MShaderParam* pParam = new MShaderParam();
			*pParam = *vNewParams[j];
			pParam->pBuffer = nullptr;
			m_pEngine->GetDevice()->GenerateShaderParamBuffer(pParam);
			vParams.push_back(pParam);
		}
	}

}

void MMaterial::RecompileShaderTextureParam(std::vector<MShaderTextureParam*>& vParams, std::vector<MResourceHolder*>& vResHolders, std::vector<MShaderTextureParam*>& vNewParams, const MEShaderParamType& eType)
{
	for (unsigned int i = 0; i < vParams.size();)
	{
		MShaderTextureParam* pParam = vParams[i];
		if (pParam->eShaderType & eType)
			pParam->eShaderType = pParam->eShaderType ^ eType;

		if (0 == pParam->eType)
		{
			delete vResHolders[i];
			vParams[i] = nullptr;
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
			if (vParams[i]->strName == vNewParams[j]->strName)
			{
				vParams[i]->eShaderType |= eType;
				bFinded = true;
				break;
			}
		}

		if (!bFinded)
		{
			MShaderTextureParam* pParam = new MShaderTextureParam();
			*pParam = *vNewParams[j];
			pParam->eShaderType = eType;
			vParams.push_back(pParam);
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

	m_vTextureParams.clear();
	for (MResourceHolder* pHolder : m_vTextureResHolder)
	{
		if (pHolder)
			delete pHolder;
	}
	m_vTextureResHolder.clear();

}
