#include "MTextureCubeResource.h"

#include "MIDevice.h"
#include "MEngine.h"

#include "MTextureResource.h"
#include "MResourceManager.h"

M_RESOURCE_IMPLEMENT(MTextureCubeResource, MResource)

MTextureCubeResource::MTextureCubeResource()
	: MResource()
	, m_pTextureCube(nullptr)
	, m_vTextures()
{
	m_unResourceType = MResourceManager::MEResourceType::Texture;
	m_pTextureCube = new MTextureCube();
	memset(m_vTextures, 0, sizeof(MResourceKeeper*) * 6);
}

MTextureCubeResource::~MTextureCubeResource()
{
	m_pTextureCube->DestroyTexture(m_pEngine->GetDevice());
	delete m_pTextureCube;
	m_pTextureCube = nullptr;
}

void MTextureCubeResource::OnDelete()
{
	for (int i = 0; i < 6; ++i)
	{
		m_vTextures[i].SetResource(nullptr);
	}

	MResource::OnDelete();
}

void MTextureCubeResource::SetTextures(MTextureResource* vTexs[6])
{
	for (int i = 0; i < 6; ++i)
	{
		m_vTextures[i].SetResource(nullptr);

		if (vTexs[i])
		{
			m_vTextures[i].SetResource(vTexs[i]);
			m_vTextures[i].SetResChangedCallback([this](const uint32_t& eReloadType){
				//TODO update TexturesCube.
				return true;
			});

			m_pTextureCube->SetTexture(vTexs[i]->GetTextureTemplate(), MTextureCube::MECubeFace(i));
		}

	}
}

bool MTextureCubeResource::Load(const MString& strResourcePath)
{
	//TODO 
	return true;
}
