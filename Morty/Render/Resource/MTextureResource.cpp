#include "MTextureResource.h"

#include "MEngine.h"
#include "MIDevice.h"

#include "MRenderSystem.h"

#include "spot.hpp"

MORTY_CLASS_IMPLEMENT(MTextureResource, MResource)

MTextureResource::MTextureResource()
	: MResource()
	, m_texture()
	, m_aByteData(nullptr)
{
	m_texture.SetMipmapsEnable(true);
	m_texture.SetReadable(false);
	m_texture.SetTextureLayout(METextureLayout::ERGBA8);
	m_texture.SetRenderUsage(METextureRenderUsage::EUnknow);
	m_texture.SetShaderUsage(METextureShaderUsage::ESampler);
}

MTextureResource::~MTextureResource()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_texture.DestroyBuffer(pRenderSystem->GetDevice());
}

void MTextureResource::LoadFromMemory(MByte* aByteData, const uint32_t& unWidth, const uint32_t& unHeight, const int& format/* = 32*/)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();


	m_texture.DestroyBuffer(pRenderSystem->GetDevice());

	m_texture.SetSize(Vector2(unWidth, unHeight));

	if (m_aByteData)
	{
		delete[] m_aByteData;
		m_aByteData = nullptr;
	}

	m_aByteData = new MByte[unWidth * unHeight * 4];

	const MByte* data8 = aByteData;

	if (format == 24)
	{
		for (size_t i = 0, pix = 0, e = unWidth * unHeight; i < e; ++i, pix += 4)
		{
			m_aByteData[pix + 0] = *data8++;
			m_aByteData[pix + 1] = *data8++;
			m_aByteData[pix + 2] = *data8++;
			m_aByteData[pix + 3] = 255;
		}
	}
	else if (format == 32)
	{
		for (size_t i = 0, pix = 0, e = unWidth * unHeight; i < e; ++i, pix += 4)
		{
			m_aByteData[pix + 0] = *data8++;
			m_aByteData[pix + 1] = *data8++;
			m_aByteData[pix + 2] = *data8++;
			m_aByteData[pix + 3] = *data8++;
		}
	}

	m_texture.GenerateBuffer(pRenderSystem->GetDevice(), m_aByteData);

}

void MTextureResource::OnDelete()
{
	MResource::OnDelete();

	if (m_aByteData)
	{
		delete[] m_aByteData;
		m_aByteData = nullptr;
	}


	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_texture.DestroyBuffer(pRenderSystem->GetDevice());
}

bool MTextureResource::Load(const MString& strResourcePath)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	std::ifstream ifs(strResourcePath.c_str(), std::ios::binary);
	if (!ifs.good())
	{
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
		GetEngine()->GetLogger()->Error("Load Texture Error: %s", error.c_str());
		return false;
	}

	if (data.empty())
	{
		GetEngine()->GetLogger()->Error("Load Texture Error: data empty.");
		return false;
	}

	LoadFromMemory(data.data(), unWidth, unHeight, comp * 8);
	
	return true;
}

bool MTextureResource::SaveTo(const MString& strResourcePath)
{
	return false;
}
