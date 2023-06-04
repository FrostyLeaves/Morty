#include "Resource/MShaderResource.h"
#include "Math/MMath.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Material/MShader.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MFileHelper.h"

MORTY_CLASS_IMPLEMENT(MShaderResource, MResource)

MShader* MShaderResource::GetShaderByIndex(const int& nIndex)
{
	int nSize = m_vShaders.size() - 1;
	return m_vShaders[MMath::Clamp(nIndex, 0, nSize)];
}

int MShaderResource::FindShaderByMacroParam(const MShaderMacro& macro)
{
	auto pShaderData = static_cast<MShaderResourceData*>(m_pResourceData.get());

	int nSize = m_vShaders.size();
	for (int i = 0 ; i < nSize; ++i)
	{
		if (m_vShaders[i]->m_ShaderMacro.Compare(macro))
			return i;
	}

	MShader* pNewShader = new MShader();
	pNewShader->m_eShaderType = pShaderData->eShaderType;
	pNewShader->m_strShaderPath = pShaderData->strShaderPath;
	pNewShader->m_ShaderMacro = macro;
	m_vShaders.push_back(pNewShader);

	return m_vShaders.size() - 1;
}

MEShaderType MShaderResource::GetShaderType() const
{
	if (auto ptr = static_cast<MShaderResourceData*>(m_pResourceData.get()))
	{
		return ptr->eShaderType;
	}

	return MEShaderType::ENone;
}

bool MShaderResource::Load(std::unique_ptr<MResourceData>& pResourceData)
{
	auto pShaderData = static_cast<MShaderResourceData*>(pResourceData.get());

	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();

	for (MShader* pShader : m_vShaders)
	{
		pShader->CleanShader(pRenderSystem->GetDevice());

		pShader->m_strShaderPath = pShaderData->strShaderPath;
		pShader->m_eShaderType = pShaderData->eShaderType;
	}

	m_pResourceData = std::move(pResourceData);

	return true;
}

bool MShaderResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
	return false;
}

void MShaderResource::OnDelete()
{
	MRenderSystem* pRenderSystem = m_pEngine->FindSystem<MRenderSystem>();
	MORTY_ASSERT(pRenderSystem);

	for (MShader* pShader : m_vShaders)
	{
		pShader->CleanShader(pRenderSystem->GetDevice());
		delete pShader;
		pShader = nullptr;
	}

	m_vShaders.clear();


	MResource::OnDelete();
}

std::shared_ptr<MResource> MShaderResourceLoader::Create(MResourceSystem* pManager)
{
	return pManager->CreateResource<MShaderResource>();
}

std::unique_ptr<MResourceData> MShaderResourceLoader::LoadResource(const MString& svFullPath, const MString& svPath)
{
	std::unique_ptr<MShaderResourceData> pResourceData = std::make_unique<MShaderResourceData>();

	const MString strPathSuffix = MResource::GetSuffix(svFullPath);

	if (strPathSuffix == MRenderGlobal::SUFFIX_VERTEX_SHADER)
	{
		pResourceData->eShaderType = MEShaderType::EVertex;
	}
	else if (strPathSuffix == MRenderGlobal::SUFFIX_PIXEL_SHADER)
	{
		pResourceData->eShaderType = MEShaderType::EPixel;
	}
	else
	{
		pResourceData->eShaderType = MEShaderType::ECompute;
	}

	pResourceData->strShaderPath = svFullPath;
	return pResourceData;
}

void MShaderResourceData::LoadBuffer(const std::vector<MByte>& buffer)
{
	
}

std::vector<MByte> MShaderResourceData::SaveBuffer() const
{
	return std::vector<MByte>();
}
