#include "MMaterial.h"
#include "MShader.h"
#include "MShaderResource.h"

MMaterial::MMaterial()
	: m_pVertexShader(nullptr)
	, m_pPixelShader(nullptr)
	, m_pNextPass(nullptr)
{

}

MMaterial::~MMaterial()
{

}

bool MMaterial::LoadVertexShader(MResource* pResource)
{
	if (MShaderResource* pShaderResource = dynamic_cast<MShaderResource*>(pResource))
	{
		if (MShader::MEShaderType::Vertex == pShaderResource->GetShaderTemplate()->GetType())
		{
			m_pVertexShader = pShaderResource;
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
			m_pPixelShader = pShaderResource;
			return true;
		}
	}

	return false;
}

MShader* MMaterial::GetVertexShader()
{
	if (m_pVertexShader)
		return m_pVertexShader->GetShaderTemplate();
	return nullptr;
}

MShader* MMaterial::GetPixelShader()
{
	if (m_pPixelShader)
		return m_pPixelShader->GetShaderTemplate();
	return nullptr;
}
