/**
 * @File         MTexture2DArrayBatcher
 *
 * @Created      2025-01-12
 *
 * @Author       DoubleYe
**/

#pragma once

#include "ITextureBatcher.h"
#include "Utility/MIDPool.h"

#include <mutex>
#include <unordered_set>

namespace morty
{

class MORTY_API MTexture2DArrayBatcher : public ITextureBatcher
{
public:
                              MTexture2DArrayBatcher()          = default;
    ~                         MTexture2DArrayBatcher() override = default;


    bool                      IsInitialized() const override { return m_initialized; }
    void                      Initialize(MIDevice* device, const MTextureBatcherConfig& config) override;
    void                      Release(MIDevice* device) override;

    MTextureBatchResult       RegisterTexture(const MTexturePtr& texture) override;
    void                      UnregisterTexture(const MTexturePtr& texture) override;
    void                      RenderThreadUpdate(MIDevice* device) override;

    [[nodiscard]] MTexturePtr GetTextureArray() const override { return m_textureArray; }
    [[nodiscard]] uint32_t    GetLayerCount() const override { return m_currentCapacity; }
    [[nodiscard]] uint32_t    GetUsedCount() const override;
    [[nodiscard]] Vector2i    GetTextureSize(int32_t layerIndex) const override;

private:
    // Create or resize the texture array
    void                          CreateTextureArray(MIDevice* device);

    // Find appropriate mip level for target size
    // Returns mip level index and actual size at that level
    std::pair<uint32_t, Vector2i> FindMipLevelForTargetSize(uint32_t width, uint32_t height) const;

    // Get bytes per pixel for current format
    size_t                        GetBytesPerPixel() const;

private:
    MTextureBatcherConfig    m_config;
    MTexturePtr              m_textureArray;

    // Layer management
    MReusableIDPool<int32_t> m_layerIdPool;

    size_t                   m_currentMaxCount = 0;
    size_t                   m_currentCapacity = 0;

    // Per-layer GPU texture storage (for GPU-side registration via RegisterTexture)
    // Key: source texture, Value: layer info including index, mip level, and actual size
    struct GPULayerInfo {
        int32_t  layerIndex = 0; // Index in the texture array
        uint32_t mipLevel   = 0; // Mip level to copy from
        Vector2i actualSize = {};// Size at that mip level
        bool     waitUpload = false;
    };
    std::unordered_map<MTexturePtr, GPULayerInfo> m_gpuLayerData;

    // Track if GPU texture needs update
    bool                                          m_dirty       = false;
    bool                                          m_initialized = false;
};

}// namespace morty
