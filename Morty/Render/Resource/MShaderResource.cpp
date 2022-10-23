#include "Resource/MShaderResource.h"
#include "Math/MMath.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Material/MShader.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

MORTY_CLASS_IMPLEMENT(MShaderResource, MResource)

MShaderResource::MShaderResource()
	: MResource()
	, m_eShaderType(MEShaderType::ENone)
	, m_strShaderPath()
{
}

MShaderResource::~MShaderResource()
{
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
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	assert(pRenderSystem);

	for (MShader* pShader : m_vShaders)
	{
		pShader->CleanShader(pRenderSystem->GetDevice());
		delete pShader;
		pShader = nullptr;
	}

	m_vShaders.clear();


	MResource::OnDelete();
}

bool MShaderResource::Load(const MString& strResourcePath)
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	MString strPathSuffix = MResource::GetSuffix(strResourcePath);

	if (strPathSuffix == MRenderGlobal::SUFFIX_VERTEX_SHADER)
		m_eShaderType = MEShaderType::EVertex;
	else if (strPathSuffix == MRenderGlobal::SUFFIX_PIXEL_SHADER)
		m_eShaderType = MEShaderType::EPixel;
	else
		m_eShaderType = MEShaderType::ECompute;


	m_strShaderPath = strResourcePath;
	for (MShader* pShader : m_vShaders)
	{
		pShader->CleanShader(pRenderSystem->GetDevice());

		pShader->m_strShaderPath = strResourcePath;
		pShader->m_eShaderType = m_eShaderType;
	}

	return true;
}
