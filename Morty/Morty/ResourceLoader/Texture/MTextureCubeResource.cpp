#include "MTextureCubeResource.h"

#include "MIDevice.h"
#include "MEngine.h"

#include "MTextureResource.h"

MTextureCubeResource::MTextureCubeResource()
	: MResource()
	, m_pTextureCube(nullptr)
	, m_vTextures()
{

	m_pTextureCube = new MTextureCube();
	memset(m_vTextures, 0, sizeof(MResourceHolder*) * 6);
}

MTextureCubeResource::~MTextureCubeResource()
{
	delete m_pTextureCube;
	m_pTextureCube = nullptr;

	for (int i = 0; i < 6; ++i)
	{
		if (m_vTextures[i])
		{
			delete m_vTextures[i];
			m_vTextures[i] = nullptr;
		}
	}
}

void MTextureCubeResource::SetTextures(MTextureResource* vTexs[6])
{
	for (int i = 0; i < 6; ++i)
	{
		if (m_vTextures[i])
		{
			delete m_vTextures[i];
			m_vTextures[i] = nullptr;
		}

		if (vTexs[i])
		{
			m_vTextures[i] = new MResourceHolder(vTexs[i]);
			m_vTextures[i]->SetResChangedCallback([this](){
				//TODO update TexturesCube.
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
