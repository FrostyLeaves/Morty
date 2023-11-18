#include "Resource/MTextureResource.h"

#include "MTextureResourceUtil.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Utility/MFileHelper.h"

#include "Flatbuffer/MTextureResource_generated.h"

#define FREE_MEMORY_AFTER_UPLOAD false

MORTY_CLASS_IMPLEMENT(MTextureResource, MResource)


template <typename ByteType>
MByte* Malloc(const size_t& nSize)
{
	return new MByte[nSize * sizeof(ByteType)];
}


MTextureResource::MTextureResource()
	: MResource()
	, m_pTexture(std::make_shared<MTexture>())
	, m_pResourceData(std::make_unique<MTextureResourceData>())
{
	m_pTexture->SetMipmapsEnable(true);
	m_pTexture->SetReadable(false);
	m_pTexture->SetTextureLayout(METextureLayout::ERGBA_UNORM_8);
	m_pTexture->SetRenderUsage(METextureWriteUsage::EUnknow);
	m_pTexture->SetShaderUsage(METextureReadUsage::EPixelSampler);
}

MTextureResource::~MTextureResource()
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_pTexture->DestroyBuffer(pRenderSystem->GetDevice());
}

void MTextureResource::OnDelete()
{
	MResource::OnDelete();

	m_pResourceData = nullptr;

	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	m_pTexture->DestroyBuffer(pRenderSystem->GetDevice());
}

bool MTextureResource::Load(std::unique_ptr<MResourceData>&& pResourceData)
{
	const MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	auto pTextureData = static_cast<MTextureResourceData*>(pResourceData.get());
	m_pTexture->SetName(pTextureData->strTextureName);
	m_pTexture->SetSize(Vector2i(pTextureData->nWidth, pTextureData->nHeight));
	m_pTexture->SetImageLayerNum(pTextureData->nImageLayerNum);
	m_pTexture->SetTextureType(pTextureData->eTextureType);
	m_pTexture->SetTextureLayout(GetTextureLayout(pTextureData->nChannel, pTextureData->ePixelFormat));
	m_pTexture->GenerateBuffer(pRenderSystem->GetDevice(), pTextureData->aByteData.data());

	if (m_bReadable)
	{
		m_pResourceData = std::move(pResourceData);
	}

	return true;
}

bool MTextureResource::SaveTo(std::unique_ptr<MResourceData>& pResourceData)
{
	MORTY_ASSERT(m_pResourceData);

	if (m_pResourceData)
	{
	    pResourceData = std::make_unique<MTextureResourceData>(*static_cast<MTextureResourceData*>(m_pResourceData.get()));
		return true;
	}

	return false;
}

void MTextureResource::CreateCubeMapRenderTarget(const uint32_t& nWidth, const uint32_t& nHeight, uint32_t nChannel, const METextureLayout& eLayout, const bool& bMipmapEnable)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	if (nChannel == 2 || nChannel == 3)
		nChannel = 4;

	m_pTexture->SetName("CubeMapRenderTarget");
	m_pTexture->SetReadable(true);
	m_pTexture->SetTextureLayout(eLayout);
	m_pTexture->SetSize(Vector2i(nWidth, nHeight));
	m_pTexture->SetRenderUsage(METextureWriteUsage::ERenderBack);
	m_pTexture->SetShaderUsage(METextureReadUsage::EPixelSampler);
	m_pTexture->SetTextureType(METextureType::ETextureCube);
	m_pTexture->SetImageLayerNum(6);
	m_pTexture->SetMipmapsEnable(bMipmapEnable);

	m_pTexture->GenerateBuffer(pRenderSystem->GetDevice());
}

METextureLayout MTextureResource::GetTextureLayout() const
{
	if (m_pTexture)
	{
		return m_pTexture->GetTextureLayout();
	}

	return METextureLayout::E_UNKNOW;
}
MTexturePixelFormat MTextureResource::GetPixelFormat() const
{
	auto ptr = static_cast<MTextureResourceData*>(m_pResourceData.get());
    return ptr->ePixelFormat;
}
size_t MTextureResource::GetChannel() const
{
	auto ptr = static_cast<MTextureResourceData*>(m_pResourceData.get());
	return ptr->nChannel;
}
size_t MTextureResource::GetWidth() const
{
	if (m_pTexture)
	{
		return m_pTexture->GetSize().x;
	}

	return 0;
}

size_t MTextureResource::GetHeight() const
{
	if (m_pTexture)
	{
		return m_pTexture->GetSize().y;
	}

	return 0;
}

const MByte* MTextureResource::GetRawData() const
{
	MORTY_ASSERT(m_pResourceData);

	if (auto ptr = static_cast<MTextureResourceData*>(m_pResourceData.get()))
	{
		return ptr->aByteData.data();
	}

	return nullptr;
}

METextureLayout MTextureResource::GetTextureLayout(const uint32_t& nChannel, const MTexturePixelFormat& format)
{
	METextureLayout eResult = METextureLayout::E_UNKNOW;
	if (MTexturePixelFormat::Byte8 == format)
	{
		static const std::array<METextureLayout, 4> sTextureLayout = {
			METextureLayout::ER_UNORM_8,
			METextureLayout::E_UNKNOW,
			METextureLayout::ERGB_UNORM_8,
			METextureLayout::ERGBA_UNORM_8,
		};

		if (0 <= nChannel && nChannel <= 4)
		{
			eResult = sTextureLayout[nChannel - 1];
		}
	}
	else if (MTexturePixelFormat::Float32 == format)
	{
		static const std::array<METextureLayout, 4> sTextureLayout = {
			METextureLayout::ER_FLOAT_32,
			METextureLayout::E_UNKNOW,
			METextureLayout::E_UNKNOW,
			METextureLayout::ERGBA_FLOAT_32,
		};

		if (0 <= nChannel && nChannel <= 4)
		{
			eResult = sTextureLayout[nChannel - 1];
		}
	}

	MORTY_ASSERT(METextureLayout::E_UNKNOW != eResult);

	return eResult;
}

flatbuffers::Offset<void> MTextureResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	auto fbTextureData = fbb.CreateVector<int8_t>(reinterpret_cast<const int8_t*>(aByteData.data()), aByteData.size());

	mfbs::MTextureResourceBuilder builder(fbb);

	builder.add_width(nWidth);
	builder.add_height(nHeight);
	builder.add_channel(nChannel);
	builder.add_pixel_format(static_cast<mfbs::MTexturePixelFormat>(ePixelFormat));
	builder.add_data(fbTextureData);

	return builder.Finish().Union();
}

void MTextureResourceData::Deserialize(const void* pBufferPointer)
{
	const mfbs::MTextureResource* fbData = mfbs::GetMTextureResource(pBufferPointer);
    nWidth = fbData->width();
    nHeight = fbData->height();
    nChannel = fbData->channel();
    ePixelFormat = static_cast<MTexturePixelFormat>(fbData->pixel_format());
    auto pData = fbData->data();
	aByteData.resize(pData->size());
	memcpy(aByteData.data(), pData->data(), pData->size());

}

MTextureImportInfo::MTextureImportInfo()
	: ePixelFormat(MTexturePixelFormat::Byte8)
{
}

MTextureImportInfo::MTextureImportInfo(MTexturePixelFormat pixelFormat)
	: ePixelFormat(pixelFormat)
{
}

const MType* MTextureResourceLoader::ResourceType() const
{
	return MTextureResource::GetClassType();
}

std::unique_ptr<MResourceData> MTextureResourceLoader::LoadResource(const MString& svFullPath)
{
	//m_strResourcePath = strResourcePath;
	if ("mtex" == MResource::GetSuffix(svFullPath))
	{
		std::unique_ptr<MTextureResourceData> pResourceData = std::make_unique<MTextureResourceData>();
		std::vector<MByte> data;
		MFileHelper::ReadData(svFullPath, data);

		flatbuffers::FlatBufferBuilder fbb;
		fbb.PushBytes((const uint8_t*)data.data(), data.size());

		pResourceData->Deserialize(fbb.GetCurrentBufferPointer());
		pResourceData->strTextureName = svFullPath;
		return pResourceData;
	}
	else
	{
		MTextureImportInfo importInfo;
		importInfo.ePixelFormat = MTexturePixelFormat::Byte8;

		return MTextureResourceUtil::ImportTexture(svFullPath, importInfo);
	}

	return nullptr;
}
