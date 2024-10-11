#include "MTextureConverter.h"

#include "MTexture_generated.h"
#include "Engine/MEngine.h"
#include "Resource/MTextureResourceUtil.h"
#include "Utility/MFileHelper.h"
#include "System/MResourceSystem.h"

using namespace morty;

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


void MTextureConverter::ConvertSingleChannel(MTextureResourceData* pTextureData, size_t nChannel)
{
    const auto& rawMipmaps = pTextureData->vMipmaps;
    auto eFormat = pTextureData->eFormat;
    size_t nWidth = pTextureData->nWidth;
    size_t nHeight = pTextureData->nHeight;

    std::vector<std::vector<MByte>> convertData;

    for (size_t nMipIdx = 0; nMipIdx < rawMipmaps.size(); ++nMipIdx)
    {
        if (morty::METextureFormat::UNorm_RGBA8 == eFormat)
        {
            convertData[nMipIdx] = ConvertSingleChannelData<MByte>(rawMipmaps[nMipIdx].data(), nWidth, nHeight, nChannel);
        }
        else if (morty::METextureFormat::Float_RGBA32 == eFormat)
        {
            convertData[nMipIdx] = ConvertSingleChannelData<float>(rawMipmaps[nMipIdx].data(), nWidth, nHeight, nChannel);
        }
        else
        {
            MORTY_ASSERT(false);
            return;
        }

        nWidth = nWidth / 2;
        nHeight = nHeight / 2;
    }

    pTextureData->vMipmaps = convertData;
    pTextureData->eFormat = eFormat;
}