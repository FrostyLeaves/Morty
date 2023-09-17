#include "MTextureConverter.h"

#include "Engine/MEngine.h"
#include "Resource/MTextureResourceUtil.h"
#include "Utility/MFileHelper.h"
#include "System/MResourceSystem.h"

template <typename TYPE>
std::vector<MByte> ConvertSingleChannelData(const MByte* vData, size_t nWidth, size_t nHeight, size_t nChannel)
{
    MORTY_ASSERT(nChannel < 4);

    std::vector<MByte> result(nWidth * nHeight * sizeof(TYPE));

    const TYPE* pData = reinterpret_cast<const TYPE*>(vData);
    TYPE* pResult = reinterpret_cast<TYPE*>(result.data());
    for (size_t w = 0; w < nWidth; ++w)
    {
        for (size_t h = 0; h < nHeight; ++h)
        {
            pResult[w * nHeight + h] = pData[(w * nHeight + h) * 4 + nChannel];
        }
    }

    return result;
}


std::shared_ptr<MTextureResource> MTextureConverter::ConvertSingleChannel(std::shared_ptr<MTextureResource> pTexture, size_t nChannel)
{
    const auto& rawData = pTexture->GetRawData();
    auto ePixelFormat = pTexture->GetPixelFormat();
    const size_t nWidth = pTexture->GetWidth();
    const size_t nHeight = pTexture->GetHeight();

    std::vector<MByte> convertData;
    
    if (MTexturePixelFormat::Byte8 == ePixelFormat)
    {
        convertData = ConvertSingleChannelData<MByte>(rawData, nWidth, nHeight, nChannel);
    }
    else if (MTexturePixelFormat::Float32 == ePixelFormat)
    {
        convertData = ConvertSingleChannelData<float>(rawData, nWidth, nHeight, nChannel);
    }
    else
    {
        MORTY_ASSERT(false);
        return nullptr;
    }

    MEngine* pEngine = pTexture->GetEngine();
    MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

    MString strResourcePath = pTexture->GetResourcePath();
    MString strFileName = MFileHelper::GetFileName(strResourcePath) + MStringUtil::ToString(nChannel);

    MString strNewResourcePath = MFileHelper::ReplaceFileName(strResourcePath, strFileName);

    auto pResult = pResourceSystem->CreateResource<MTextureResource>(strNewResourcePath);

    pResult->Load(MTextureResourceUtil::LoadFromMemory(strNewResourcePath, convertData.data(), nWidth, nHeight, 1, ePixelFormat));

    return pResult;
}