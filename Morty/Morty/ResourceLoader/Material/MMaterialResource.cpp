#include "MMaterialResource.h"
#include "MShaderResource.h"
#include "MShader.h"

MMaterialResource::MMaterialResource()
{

}

MMaterialResource::~MMaterialResource()
{

}

bool MMaterialResource::Load(const MString& strResourcePath)
{

	return false;
}

bool MMaterialResource::LoadVertexShader(MResource* pResource)
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

bool MMaterialResource::LoadPixelShader(MResource* pResource)
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
