#include "MMaterialResource.h"
#include "MShaderResource.h"
#include "MEngine.h"
#include "MShader.h"
#include "MMaterial.h"

MMaterialResource::MMaterialResource()
	: m_pVertexShader(nullptr)
	, m_pPixelShader(nullptr)
	, m_pVertexResource(nullptr)
	, m_pPixelResource(nullptr)
	, m_pMaterial(nullptr)
{
	
}

MMaterialResource::~MMaterialResource()
{
	m_pVertexShader = nullptr;
	m_pPixelShader = nullptr;

	if (m_pVertexResource)
	{
		delete m_pVertexResource;
		m_pVertexResource = nullptr;
	}
	if (m_pPixelResource)
	{
		delete m_pPixelResource;
		m_pPixelResource = nullptr;
	}
// 	if (m_pMaterial)
// 	{		//TODO: Circular reference!
// 		delete m_pMaterial;
// 		m_pMaterial = nullptr;
// 	}
}

bool MMaterialResource::Load(const MString& strResourcePath)
{
	//TODO 
	return false;
}

void MMaterialResource::OnCreated()
{
	m_pMaterial = m_pEngine->GetObjectManager()->CreateObject<MMaterial>();
	m_pMaterial->Load(this);
}

bool MMaterialResource::LoadVertexShader(MResource* pResource)
{
	if (MShaderResource* pShaderResource = dynamic_cast<MShaderResource*>(pResource))
	{
		if (MShader::MEShaderType::Vertex == pShaderResource->GetShaderTemplate()->GetType())
		{
			m_pVertexShader = pShaderResource->GetShaderTemplate();
			if (m_pVertexResource)
				delete m_pVertexResource;
			m_pVertexResource = new MResourceHolder(pResource);
			m_pVertexResource->SetResChangedCallback([this](const unsigned int& eReloadType){
				OnReload(EResReloadType::EVertex);
				return true;
			});

			OnReload(EResReloadType::EVertex);
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
			m_pPixelShader = pShaderResource->GetShaderTemplate();
			if (m_pPixelResource)
				delete m_pPixelResource;
			m_pPixelResource = new MResourceHolder(pResource);
			m_pPixelResource->SetResChangedCallback([this](const unsigned int& eReloadType){
				OnReload(EResReloadType::EPixel);
				return true;
			});

			OnReload(EResReloadType::EPixel);
			return true;
		}
	}

	return false;
}
