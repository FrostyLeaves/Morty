#include "MMaterial.h"
#include "MShader.h"
#include "MShaderResource.h"
#include "MMaterialResource.h"
#include "MTextureResource.h"
#include "MTextureCubeResource.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MIRenderer.h"

#include "MVariant.h"

#include <algorithm>

MTypeIdentifierImplement(MMaterial, MObject)

MMaterial::MMaterial()
	: MObject()
	, m_pMaterialResource(nullptr)
	, m_pVertexShader(nullptr)
	, m_pPxielShader(nullptr)
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
			RecompileShaderParams(m_vShaderParams, m_pPxielShader->GetBuffer()->m_vShaderParamsTemplate, MEShaderParamType::EPixel);
			RecompileShaderTextureParam(m_vTextureParams, m_vTextureResHolder, m_pPxielShader->GetBuffer()->m_vTextureParamsTemplate, MEShaderParamType::EPixel);
		}
	}
}

bool MMaterial::Load(MMaterialResource* pResource)
{
	Unload();

	auto UseResourceFunction = [this](const unsigned int& eReloadType){
		if (MMaterialResource* pMatResource = static_cast<MMaterialResource*>(m_pMaterialResource->GetResource()))
		{
			m_pVertexShader = pMatResource->GetVertexShader();
			m_pPxielShader = pMatResource->GetPixelShader();
		}

		if (m_pVertexShader && nullptr == m_pVertexShader->GetBuffer())
		{
			if (!m_pVertexShader->CompileShader(m_pEngine->GetDevice()))
				return false;
		}
		if (m_pPxielShader && nullptr == m_pPxielShader->GetBuffer())
		{
			if (!m_pPxielShader->CompileShader(m_pEngine->GetDevice()))
				return false;
		}

		if(MMaterialResource::EResReloadType::EPixel != eReloadType)
			CompileShaderParams(MEShaderParamType::EVertex);
		if (MMaterialResource::EResReloadType::EVertex != eReloadType)
			CompileShaderParams(MEShaderParamType::EPixel);

		return true;
	};

	if (MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(pResource))
	{
		if (m_pMaterialResource)
			delete m_pMaterialResource;
		m_pMaterialResource = new MResourceHolder(pResource);
		m_pMaterialResource->SetResChangedCallback(UseResourceFunction);

		return UseResourceFunction(MResource::EResReloadType::EDefault);
	}

	return false;
}

MResource* MMaterial::GetResource()
{
	return m_pMaterialResource->GetResource();
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

	if (m_pMaterialResource)
	{
		delete m_pMaterialResource;
		m_pMaterialResource = nullptr;
	}

	m_pVertexShader = nullptr;
	m_pPxielShader = nullptr;

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


	/*
	std::vector<MShaderParam*> vParamsTemp(vNewParams.size());
	for (unsigned int i = vNewParams.size() - 1; i >= 0; --i)
	{
		vParamsTemp[i] = new MShaderParam();
		*vParamsTemp[i] = *vNewParams[i];
	}

	auto funcSortFunc = [](MShaderParam* p1, MShaderParam* p2) {
		return p1->strName < p2->strName;
	};

	std::sort(vParams.begin(), vParams.end(), funcSortFunc);
	std::sort(vParamsTemp.begin(), vParamsTemp.end(), funcSortFunc);

	unsigned int paramsSize = vParams.size();
	unsigned int paramsTempSize = vParamsTemp.size();

	unsigned int i = 0;
	for (unsigned int j = 0; j < paramsTempSize; ++j)
	{
		while (i < paramsSize && vParams[i]->strName < vParamsTemp[j]->strName)
		{
			MShaderParam* pParam = new MShaderParam();
			*pParam = *vParams[i];
			vParamsTemp.push_back(pParam);
;
			++i;
		}

		if (i >= paramsSize)
			break;

		if (vParams[i]->strName == vParamsTemp[j]->strName)
		{
			vParamsTemp[j]->var.MergeFrom(vParams[i]->var);
			vParamsTemp[j]->eType |= vParams[i]->eType;
		}

		vParamsTemp[j]->pBuffer = nullptr;
	}

	while (i < paramsSize)
	{
		vParamsTemp.push_back(vParams[i]);
		++i;
	}

	for (MShaderParam* pParam : vParams)
	{
		m_pEngine->GetDevice()->DestroyShaderParamBuffer(pParam);
		delete pParam;
	}

	vParams = vParamsTemp;

	for (MShaderParam* pParam : vParams)
		m_pEngine->GetDevice()->GenerateShaderParamBuffer(pParam);
	*/
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
