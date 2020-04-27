#include "MShaderResource.h"
#include "MShader.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MResourceManager.h"

MShaderResource::MShaderResource()
{
	m_unResourceType = MResourceManager::MEResourceType::Shader;
	m_pShaderTemplate = new MShader();
}

MShaderResource::~MShaderResource()
{
	m_pShaderTemplate->CleanShader(m_pEngine->GetDevice());
	delete m_pShaderTemplate;
}

bool MShaderResource::Load(const MString& strResourcePath)
{
	m_pShaderTemplate->CleanShader(m_pEngine->GetDevice());

	m_pShaderTemplate->m_strShaderPath = strResourcePath;
	m_pShaderTemplate->m_eShaderType = MResource::GetSuffix(strResourcePath) == SUFFIX_VERTEX_SHADER ? MShader::MEShaderType::Vertex : MShader::MEShaderType::Pixel;

	return true;
}
