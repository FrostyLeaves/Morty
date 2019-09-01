#include "MMaterial.h"
#include "MShader.h"
#include "MShaderResource.h"
#include "MMaterialResource.h"
#include "MEngine.h"

MMaterial::MMaterial()
	: m_pNextPass(nullptr)
{

}

MMaterial::~MMaterial()
{

}

MShader* MMaterial::GetVertexShader()
{
	if (m_pResource && m_pResource->GetVertexShaderResource())
	{
		return m_pResource->GetVertexShaderResource()->GetShaderTemplate();
	}

	return nullptr;
}

MShader* MMaterial::GetPixelShader()
{
	if (m_pResource && m_pResource->GetPixelShaderResource())
	{
		return m_pResource->GetPixelShaderResource()->GetShaderTemplate();
	}

	return nullptr;
}

void MMaterial::CompileVertexShaderParams()
{
	MShader* pVertexShader = GetVertexShader();
	if (pVertexShader && pVertexShader->GetBuffer())
	{
		m_vVertexShaderParams.clear();
		m_vVertexShaderParams = pVertexShader->GetBuffer()->m_vShaderParamsTemplate;
	}
}

void MMaterial::CompilePixelShaderParams()
{
	MShader* pPixelShader = GetPixelShader();
	if (pPixelShader && pPixelShader->GetBuffer())
	{
		m_vPixelShaderParams.clear();
		m_vPixelShaderParams = pPixelShader->GetBuffer()->m_vShaderParamsTemplate;
	}
}

bool MMaterial::Load(MResource* pResource)
{
	if (MMaterialResource* pMaterialRes = dynamic_cast<MMaterialResource*>(pResource))
	{
		m_pResource = pMaterialRes;

		MShader* pVertexShader = GetVertexShader();
		MShader* pPixelShader = GetPixelShader();
		if (pVertexShader && nullptr == pVertexShader->GetBuffer())
		{
			pVertexShader->CompileShader(m_pEngine->GetRenderer());
			CompileVertexShaderParams();
		}
		if (pPixelShader && nullptr == pPixelShader->GetBuffer())
		{
			pPixelShader->CompileShader(m_pEngine->GetRenderer());
			CompilePixelShaderParams();
		}

		//Do smoething.
		return true;
	}

	return false;
}
