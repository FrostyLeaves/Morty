#include "MShaderResource.h"
#include "MShader.h"

MShaderResource::MShaderResource()
{
	m_pShaderTemplate = new MShader();
}

MShaderResource::~MShaderResource()
{
	delete m_pShaderTemplate;
}

bool MShaderResource::Load(const MString& strResourcePath)
{
	m_pShaderTemplate->m_strShaderPath = strResourcePath;
	m_pShaderTemplate->m_eShaderType = MResource::GetSuffix(strResourcePath) == SUFFIX_VERTEX_SHADER ? MShader::MEShaderType::Vertex : MShader::MEShaderType::Pixel;

	return true;
}
