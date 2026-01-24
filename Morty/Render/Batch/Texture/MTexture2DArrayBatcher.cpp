/**
 * @File         MTexture2DArrayBatcher
 *
 * @Created      2025-01-12
 *
 * @Author       DoubleYe
**/

#include "MTexture2DArrayBatcher.h"

#include "RHI/Abstract/MIDevice.h"

#include <algorithm>

using namespace morty;

void MTexture2DArrayBatcher::Initialize(MIDevice* device, const MTextureBatcherConfig& config)
{
    if (m_initialized) { return; }

    m_config          = config;
    m_currentCapacity = 0;
    m_initialized     = true;
    CreateTextureArray(device);
}

void MTexture2DArrayBatcher::Release(MIDevice* device)
{
    if (!m_initialized) { return; }
    if (m_textureArray) { m_textureArray->DestroyBuffer(device); }

    m_textureArray = nullptr;
    m_gpuLayerData.clear();
    m_layerIdPool     = MReusableIDPool<int32_t>();
    m_initialized     = false;
    m_dirty           = false;
    m_currentCapacity = 0;
}

MTextureBatchResult MTexture2DArrayBatcher::RegisterTexture(const MTexturePtr& texture)
{
    MTextureBatchResult result;

    if (!m_initialized)
    {
        result.errorMessage = "Batcher not initialized";
        return result;
    }

    if (!texture)
    {
        result.errorMessage = "Texture is null";
        return result;
    }

    // Validate format matches
    if (texture->GetFormat() != m_config.format)
    {
        result.errorMessage = "Format mismatch: source format does not match batcher format";
        return result;
    }

    auto findResult = m_gpuLayerData.find(texture);
    if (findResult != m_gpuLayerData.end())
    {
        result.layerIndex = findResult->second.layerIndex;
        result.actualSize = findResult->second.actualSize;
        result.success    = true;
        return result;
    }

    // Find appropriate mip level
    auto [mipLevel, actualSize] = FindMipLevelForTargetSize(static_cast<uint32_t>(texture->GetSize().x), static_cast<uint32_t>(texture->GetSize().y));

    // Validate mip level exists
    if (mipLevel >= texture->GetMipmapLevel())
    {
        // Use the smallest available mip
        mipLevel   = texture->GetMipmapLevel() > 0 ? texture->GetMipmapLevel() - 1 : 0;
        actualSize = Vector2i(std::max(1, texture->GetSize().x >> mipLevel), std::max(1, texture->GetSize().y >> mipLevel));
    }

    // Allocate a layer index
    int32_t layerIndex = m_layerIdPool.AllocateID();
    m_currentMaxCount  = std::max(m_currentMaxCount, static_cast<size_t>(layerIndex) + 1);

    // Store GPU layer info for later copy in RenderThreadUpdate
    GPULayerInfo gpuInfo{};
    gpuInfo.layerIndex = layerIndex;
    gpuInfo.mipLevel   = mipLevel;
    gpuInfo.actualSize = actualSize;
    gpuInfo.waitUpload = true;

    m_gpuLayerData[texture] = gpuInfo;
    m_dirty                 = true;

    result.success    = true;
    result.layerIndex = layerIndex;
    result.actualSize = actualSize;
    return result;
}

void MTexture2DArrayBatcher::UnregisterTexture(const MTexturePtr& texture)
{
    if (!m_initialized) { return; }

    auto it = m_gpuLayerData.find(texture);
    if (it == m_gpuLayerData.end()) { return; }

    const auto info = it->second;
    m_layerIdPool.FreeID(info.layerIndex);
    m_gpuLayerData.erase(it);

    m_dirty = true;
}

void MTexture2DArrayBatcher::RenderThreadUpdate(MIDevice* device)
{
    if (!m_initialized || !m_dirty) { return; }

    if (m_currentMaxCount > m_currentCapacity)
    {
        m_currentCapacity = m_currentMaxCount * 2;
        if (m_textureArray == nullptr) { CreateTextureArray(device); }
        else { m_textureArray->ResizeLayer(device, m_currentCapacity); }
    }

    // Copy GPU textures to array layers
    for (auto& [texture, gpuInfo]: m_gpuLayerData)
    {
        if (texture && gpuInfo.waitUpload)
        {
            device->CopyImage(
                    texture.get(),
                    m_textureArray.get(),
                    gpuInfo.mipLevel,                         // srcMip
                    0,                                        // srcSlice (source is 2D texture)
                    0,                                        // dstMip
                    static_cast<uint32_t>(gpuInfo.layerIndex),// dstSlice (array layer)
                    static_cast<uint32_t>(gpuInfo.actualSize.x),
                    static_cast<uint32_t>(gpuInfo.actualSize.y)
            );

            gpuInfo.waitUpload = false;
        }
    }

    m_dirty = false;
}

uint32_t MTexture2DArrayBatcher::GetUsedCount() const { return static_cast<uint32_t>(m_gpuLayerData.size()); }

Vector2i MTexture2DArrayBatcher::GetTextureSize(int32_t layerIndex) const
{
    for (const auto& [texture, gpuInfo]: m_gpuLayerData)
    {
        if (gpuInfo.layerIndex == layerIndex) { return gpuInfo.actualSize; }
    }

    return Vector2i(0, 0);
}

void MTexture2DArrayBatcher::CreateTextureArray(MIDevice* device)
{
    // Ensure minimum capacity
    if (m_currentCapacity == 0) { m_currentCapacity = m_config.initialCapacity; }

    // Create texture descriptor
    MTextureDesc desc;
    desc.strName         = "TextureBatchArray";
    desc.n3Size          = Vector3i(m_config.targetSize.x, m_config.targetSize.y, 1);
    desc.nLayer          = m_currentCapacity;
    desc.eTextureType    = METextureType::ETexture2DArray;
    desc.eFormat         = m_config.format;
    desc.eMipmapDataType = m_config.generateMipmaps ? MEMipmapDataType::Generate : MEMipmapDataType::Disable;
    desc.nReadUsage      = METextureReadUsageBit::EPixelSampler;
    desc.nWriteUsage     = METextureWriteUsageBit::EUnknow;

    m_textureArray = MTexture::CreateTexture(desc);
    m_textureArray->GenerateBuffer(device);
}

std::pair<uint32_t, Vector2i> MTexture2DArrayBatcher::FindMipLevelForTargetSize(uint32_t width, uint32_t height) const
{
    const auto targetWidth  = static_cast<uint32_t>(m_config.targetSize.x);
    const auto targetHeight = static_cast<uint32_t>(m_config.targetSize.y);

    // If source is smaller or equal to target, use mip level 0
    if (width <= targetWidth && height <= targetHeight) { return {0, Vector2i(width, height)}; }

    // Find the mip level where both dimensions are <= target size
    uint32_t mipLevel  = 0;
    uint32_t mipWidth  = width;
    uint32_t mipHeight = height;

    while (mipWidth > targetWidth || mipHeight > targetHeight)
    {
        ++mipLevel;
        mipWidth  = std::max(1u, mipWidth / 2);
        mipHeight = std::max(1u, mipHeight / 2);
    }

    return {mipLevel, Vector2i(mipWidth, mipHeight)};
}

size_t MTexture2DArrayBatcher::GetBytesPerPixel() const
{
    switch (m_config.format)
    {
        case METextureFormat::UNorm_R8: return 1;
        case METextureFormat::UNorm_RG8: return 2;
        case METextureFormat::UNorm_RGB8: return 3;
        case METextureFormat::UNorm_RGBA8:
        case METextureFormat::SRGB_R8G8B8A8: return 4;
        case METextureFormat::Float_R16: return 2;
        case METextureFormat::Float_RG16: return 4;
        case METextureFormat::Float_RGB16: return 6;
        case METextureFormat::Float_RGBA16: return 8;
        case METextureFormat::Float_R32: return 4;
        case METextureFormat::Float_RG32: return 8;
        case METextureFormat::Float_RGB32: return 12;
        case METextureFormat::Float_RGBA32: return 16;
        default: return 4;// Default to 4 bytes
    }
}
