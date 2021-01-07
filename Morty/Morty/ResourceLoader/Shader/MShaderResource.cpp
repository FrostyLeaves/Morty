#include "MShaderResource.h"
#include "MMath.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "Shader/MShader.h"
#include "MResourceManager.h"

M_RESOURCE_IMPLEMENT(MShaderResource, MResource)

MShaderResource::MShaderResource()
	: MResource()
	, m_eShaderType(MShader::MEShaderType::None)
	, m_strShaderPath()
{
	m_eResourceType = MEResourceType::Shader;
}

MShaderResource::~MShaderResource()
{
	for (MShader* pShader : m_vShaders)
	{
		pShader->CleanShader(m_pEngine->GetDevice());
		delete pShader;
		pShader = nullptr;
	}

	m_vShaders.clear();

}

MShader* MShaderResource::GetShaderByIndex(const int& nIndex)
{
	int nSize = m_vShaders.size() - 1;
	return m_vShaders[MMath::Clamp(nIndex, 0, nSize)];
}

int MShaderResource::FindShaderByMacroParam(const MShaderMacro& macro)
{
	int nSize = m_vShaders.size();
	for (int i = 0 ; i < nSize; ++i)
	{
		if (m_vShaders[i]->m_ShaderMacro.Compare(macro))
			return i;
	}

	MShader* pNewShader = new MShader();
	pNewShader->m_eShaderType = m_eShaderType;
	pNewShader->m_strShaderPath = m_strShaderPath;
	pNewShader->m_ShaderMacro = macro;
	m_vShaders.push_back(pNewShader);

	return m_vShaders.size() - 1;
}

void MShaderResource::OnDelete()
{
	MResource::OnDelete();
}

bool MShaderResource::Load(const MString& strResourcePath)
{
	m_eShaderType = MResource::GetSuffix(strResourcePath) == SUFFIX_VERTEX_SHADER ? MShader::MEShaderType::Vertex : MShader::MEShaderType::Pixel;
	m_strShaderPath = strResourcePath;
	for (MShader* pShader : m_vShaders)
	{
		pShader->CleanShader(m_pEngine->GetDevice());

		pShader->m_strShaderPath = strResourcePath;
		pShader->m_eShaderType = m_eShaderType;
	}

	return true;
}
