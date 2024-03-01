#include "Resource/MTextureResource.h"

#include "MAstcTextureUtil.h"
#include "MDdsTextureUtil.h"
#include "MTextureResourceUtil.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Utility/MFileHelper.h"

#include "Flatbuffer/MTextureResource_generated.h"

using namespace morty;

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
	m_pTexture->SetMipmapDataType(MEMipmapDataType::Generate);
	m_pTexture->SetReadable(false);
	m_pTexture->SetTextureLayout(METextureLayout::UNorm_RGBA8);
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
	m_pTexture->SetMipmapDataType(pTextureData->eMipmapDataType);
	m_pTexture->SetTextureLayout(static_cast<METextureLayout>(pTextureData->eFormat));

	m_pTexture->GenerateBuffer(pRenderSystem->GetDevice(), pTextureData->vMipmaps);

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
	m_pTexture->SetMipmapDataType(bMipmapEnable? MEMipmapDataType::Generate:MEMipmapDataType::Disable);

	m_pTexture->GenerateBuffer(pRenderSystem->GetDevice());
}

METextureLayout MTextureResource::GetTextureLayout() const
{
	if (m_pTexture)
	{
		return m_pTexture->GetTextureLayout();
	}

	auto ptr = static_cast<MTextureResourceData*>(m_pResourceData.get());
	return static_cast<METextureLayout>(ptr->eFormat);
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
		return ptr->vMipmaps[0].data();
	}

	return nullptr;
}

flatbuffers::Offset<void> MTextureResourceData::Serialize(flatbuffers::FlatBufferBuilder& fbb) const
{
	std::vector<flatbuffers::Offset<morty::fbs::MTextureMipmapData>> vMipmapOffset(vMipmaps.size());

	std::transform(vMipmaps.begin(), vMipmaps.end(), vMipmapOffset.begin(), [&fbb](const auto& range)
		{
			auto fbBuffer = fbb.CreateVector<int8_t>(reinterpret_cast<const int8_t*>(range.data()), range.size());
			fbs::MTextureMipmapDataBuilder builder(fbb);
	        builder.add_buffer(fbBuffer);
			return builder.Finish();
		});

	const auto fbMipmaps = fbb.CreateVector(vMipmapOffset);

	fbs::MTextureResourceBuilder builder(fbb);

	builder.add_width(static_cast<uint32_t>(nWidth));
	builder.add_height(static_cast<uint32_t>(nHeight));
	builder.add_format(eFormat);
	builder.add_mipmap_type(eMipmapDataType);
	builder.add_mipmaps(fbMipmaps);

	return builder.Finish().Union();
}

void MTextureResourceData::Deserialize(const void* pBufferPointer)
{
	const fbs::MTextureResource* fbData = fbs::GetMTextureResource(pBufferPointer);
    nWidth = fbData->width();
    nHeight = fbData->height();
	eFormat = fbData->format();
	eMipmapDataType = fbData->mipmap_type();

	if (fbData->mipmaps())
	{
		vMipmaps.resize(fbData->mipmaps()->size());
		for (size_t nIdx = 0; nIdx < fbData->mipmaps()->size(); ++nIdx)
		{
			const auto fbBuffer = fbData->mipmaps()->Get(nIdx)->buffer();
			vMipmaps[nIdx] = std::vector<MByte>{ fbBuffer->begin(), fbBuffer->end() };
		}
	}
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

    if ("astc" == MResource::GetSuffix(svFullPath))
	{
		return MAstcTextureUtil::ImportAstcTexture(svFullPath);
	}

	if ("dds" == MResource::GetSuffix(svFullPath))
	{
		return MDdsTextureUtil::ImportDdsTexture(svFullPath);
	}
	
    return MTextureResourceUtil::ImportTexture(svFullPath, MTexturePixelType::Byte8);
}
