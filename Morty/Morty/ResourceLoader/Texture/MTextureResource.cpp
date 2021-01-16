#include "MTextureResource.h"
#include "MResourceManager.h"

#include "MIDevice.h"
#include "MEngine.h"

#include "spot.hpp"

M_RESOURCE_IMPLEMENT(MTextureResource, MResource)

MTextureResource::MTextureResource()
	:MResource()
{
	m_eResourceType = MEResourceType::Texture;
	m_pTexture = new MTexture();
}

MTextureResource::~MTextureResource()
{
	m_pTexture->DestroyBuffer(m_pEngine->GetDevice());
	delete m_pTexture;
	m_pTexture = nullptr;
}

void MTextureResource::OnDelete()
{
	MResource::OnDelete();
}

bool MTextureResource::Load(const MString& strResourcePath)
{
	std::ifstream ifs(strResourcePath.c_str(), std::ios::binary);
	if (!ifs.good())
	{
//		MLogManager::GetInstance()->Error("Load Texture Error: [path:%s] not found", strResourcePath.c_str());
		return false;
	}
	size_t unWidth = 0;
	size_t unHeight = 0;
	size_t comp;
	std::string error;

	std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	std::vector<unsigned char> data = spot::decode8(buffer.data(), buffer.size(), &unWidth, &unHeight, &comp, &error);
	if (error.size())
	{
		MLogManager::GetInstance()->Error("Load Texture Error: %s", error.c_str());
		return false;
	}

	if (data.empty())
	{
		MLogManager::GetInstance()->Error("Load Texture Error: data empty.");
		return false;
	}

	m_pTexture->DestroyBuffer(m_pEngine->GetDevice());

	m_pTexture->SetSize(Vector2(unWidth, unHeight));

	unsigned char* imgData = m_pTexture->GetImageData();

	const unsigned char* data8(data.data());

	if (comp == 3)
	{
		for (size_t i = 0, pix = 0, e = unWidth * unHeight; i < e; ++i, pix += 4)
		{
			imgData[pix + 0] = *data8++;
			imgData[pix + 1] = *data8++;
			imgData[pix + 2] = *data8++;
			imgData[pix + 3] = 255;
		}
	}
	else if (comp == 4)
	{
		for (size_t i = 0, pix = 0, e = unWidth * unHeight; i < e; ++i, pix += 4)
		{
			imgData[pix + 0] = *data8++;
			imgData[pix + 1] = *data8++;
			imgData[pix + 2] = *data8++;
			imgData[pix + 3] = *data8++;
		}
	}

	m_pTexture->GenerateBuffer(m_pEngine->GetDevice());
	
	return true;
}

bool MTextureResource::SaveTo(const MString& strResourcePath)
{
	return false;
}
