#include "MMaterial.h"
#include "MShader.h"
#include "MShaderResource.h"
#include "MMaterialResource.h"
#include "MTextureResource.h"
#include "MTextureCubeResource.h"
#include "MEngine.h"

#include "MVariable.h"

MMaterial::MMaterial()
	: m_pMaterialResource(nullptr)
	, m_pVertexShader(nullptr)
	, m_pPxielShader(nullptr)
{

}

MMaterial::~MMaterial()
{
	Unload();
}

void MMaterial::SetPixelTexutreParam(const MString& strName, MResource* pResource)
{
	for (int i = 0; i < m_vPixelTextureParams.size(); ++i)
	{
		MShaderTextureParam& param = m_vPixelTextureParams[i];
		if (strName == param.strName)
		{
			if (param.eType == ETexture2D)
			{
				if (MTextureResource* pTexResource = dynamic_cast<MTextureResource*>(pResource))
				{
					if (m_vPixelTextureResHolder[i])
						delete m_vPixelTextureResHolder[i];
					m_vPixelTextureResHolder[i] = new MResourceHolder(pResource);
					m_vPixelTextureResHolder[i]->SetResChangedCallback([&param, &pTexResource](){
						param.pTexture = pTexResource->GetTextureTemplate();
					});

					param.pTexture = pTexResource->GetTextureTemplate();
				}
			}
			else if (param.eType == ETextureCube)
			{
				if (MTextureCubeResource* pTexResource = dynamic_cast<MTextureCubeResource*>(pResource))
				{
					if (m_vPixelTextureResHolder[i])
						delete m_vPixelTextureResHolder[i];
					m_vPixelTextureResHolder[i] = new MResourceHolder(pResource);
					m_vPixelTextureResHolder[i]->SetResChangedCallback([&param, &pTexResource](){
						param.pTexture = pTexResource->GetTextureCubeTemplate();
					});

					param.pTexture = pTexResource->GetTextureCubeTemplate();
				}
			}
		}
	}
	
}

void MMaterial::CompileVertexShaderParams()
{
	if (m_pVertexShader && m_pVertexShader->GetBuffer())
	{
		m_vVertexShaderParams.clear();
		m_vVertexShaderParams = m_pVertexShader->GetBuffer()->m_vShaderParamsTemplate;
	}
}

void MMaterial::CompilePixelShaderParams()
{
	if (m_pPxielShader && m_pPxielShader->GetBuffer())
	{
		CleanTextureParams();

		m_vPixelShaderParams.clear();
		m_vPixelShaderParams = m_pPxielShader->GetBuffer()->m_vShaderParamsTemplate;
		m_vPixelTextureParams = m_pPxielShader->GetBuffer()->m_vTextureParamsTemplate;
		m_vPixelTextureResHolder.resize(m_vPixelTextureParams.size(), nullptr);
	}
}

void MMaterial::SetPixelParam(const MString& strName, const MVariable& variable)
{
	for (MShaderParam& param : m_vPixelShaderParams)
	{
		if (param.strName == strName)
		{
			if (param.var.GetType() == variable.GetType())
				param.var = variable;

			break;
		}
		else if (param.var.GetType() == MVariable::EStruct)
		{
			MStruct* pStruct = param.var.GetByType<MStruct>();
			if (MVariable* pVar = pStruct->FindMember(strName))
			{
				*pVar = variable;
				break;
			}
		}
	}
}

bool MMaterial::Load(MResource* pResource)
{
	Unload();

	auto UseResourceFunction = [this](){
		if (MMaterialResource* pMatResource = static_cast<MMaterialResource*>(m_pMaterialResource->GetResource()))
		{
			m_pVertexShader = pMatResource->GetVertexShader();
			m_pPxielShader = pMatResource->GetPixelShader();
		}

		if (m_pVertexShader && nullptr == m_pVertexShader->GetBuffer())
		{
			m_pVertexShader->CompileShader(m_pEngine->GetDevice());

		}
		if (m_pPxielShader && nullptr == m_pPxielShader->GetBuffer())
		{
			m_pPxielShader->CompileShader(m_pEngine->GetDevice());
		}

		CompileVertexShaderParams();
		CompilePixelShaderParams();
	};

	if (MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(pResource))
	{
		if (m_pMaterialResource)
			delete m_pMaterialResource;
		m_pMaterialResource = new MResourceHolder(pResource);
		m_pMaterialResource->SetResChangedCallback(UseResourceFunction);

		UseResourceFunction();

		//Do smoething.
		return true;
	}

	return false;
}

void MMaterial::Unload()
{
	CleanTextureParams();

	m_vVertexShaderParams.clear();
	m_vPixelShaderParams.clear();

	if (m_pMaterialResource)
	{
		delete m_pMaterialResource;
		m_pMaterialResource = nullptr;
	}

	m_pVertexShader = nullptr;
	m_pPxielShader = nullptr;

}

void MMaterial::CleanTextureParams()
{
	m_vPixelTextureParams.clear();
	for (MResourceHolder* pHolder : m_vPixelTextureResHolder)
	{
		if (pHolder)
			delete pHolder;
	}
	m_vPixelTextureResHolder.clear();
}
