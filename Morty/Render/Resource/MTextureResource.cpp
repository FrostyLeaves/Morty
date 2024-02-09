#include "Resource/MTextureResource.h"

#include "MAstcTextureUtil.h"
#include "MTextureResourceUtil.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Utility/MFileHelper.h"

#include "Flatbuffer/MTextureResource_generated.h"

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
	m_pTexture->SetSize(Vector3i(pTextureData->nWidth, pTextureData->nHeight, pTextureData->nDepth));
	m_pTexture->SetTextureType(pTextureData->eTextureType);
	m_pTexture->SetTextureLayout(MTextureResourceUtil::GetTextureLayout(pTextureData->eFormat));
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
	m_pTexture->SetSize(Vector3i(nWidth, nHeight, 6));
	m_pTexture->SetRenderUsage(METextureWriteUsage::ERenderBack);
	m_pTexture->SetShaderUsage(METextureReadUsage::EPixelSampler);
	m_pTexture->SetTextureType(METextureType::ETextureCube);
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

MTextureResourceFormat MTextureResource::GetFormat() const
{
	auto ptr = static_cast<MTextureResourceData*>(m_pResourceData.get());
    return ptr->eFormat;
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

flatbuffers::Offset<void> MTextureResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	auto fbTextureData = fbb.CreateVector<int8_t>(reinterpret_cast<const int8_t*>(aByteData.data()), aByteData.size());

	mfbs::MTextureResourceBuilder builder(fbb);

	builder.add_width(static_cast<uint32_t>(nWidth));
	builder.add_height(static_cast<uint32_t>(nHeight));
	builder.add_format(static_cast<mfbs::MTextureResourceFormat>(eFormat));
	builder.add_data(fbTextureData);

	return builder.Finish().Union();
}

void MTextureResourceData::Deserialize(const void* pBufferPointer)
{
	const mfbs::MTextureResource* fbData = mfbs::GetMTextureResource(pBufferPointer);
    nWidth = fbData->width();
    nHeight = fbData->height();
	eFormat = static_cast<MTextureResourceFormat>(fbData->format());
    const auto pData = fbData->data();
	aByteData.resize(pData->size());
	memcpy(aByteData.data(), pData->data(), pData->size());

}

MTextureImportInfo::MTextureImportInfo(const MTexturePixelType nPixelSize)
	: ePixelType(nPixelSize)
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
	else if ("astc" == MResource::GetSuffix(svFullPath))
	{
		return MAstcTextureUtil::ImportAstcTexture(svFullPath);
	}
	else
	{
		return MTextureResourceUtil::ImportTexture(svFullPath, MTexturePixelType::Byte8);
	}

	return nullptr;
}
