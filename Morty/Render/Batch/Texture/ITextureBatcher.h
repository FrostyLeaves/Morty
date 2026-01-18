/**
 * @File         ITextureBatcher
 *
 * @Created      2025-01-12
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Basic/MTexture.h"

namespace morty
{

class MIDevice;

// Result of texture registration
struct MORTY_API MTextureBatchResult {
    bool     success    = false;
    int32_t  layerIndex = -1;            // Index in the texture array, -1 means invalid
    Vector2i actualSize = Vector2i(0, 0);// Actual size of the texture in the array
    MString  errorMessage;
};

// Configuration for texture batcher
struct MORTY_API MTextureBatcherConfig {
    Vector2i        targetSize      = Vector2i(512, 512);
    METextureFormat format          = METextureFormat::UNorm_RGBA8;
    uint32_t        initialCapacity = 16;
    uint32_t        maxCapacity     = 4096;
    bool            generateMipmaps = true;
};

// Interface for texture batching implementations
// Combines multiple textures into a single texture array for efficient batched rendering
class MORTY_API ITextureBatcher
{
public:
    virtual ~                         ITextureBatcher() = default;


    virtual bool                     IsInitialized() const = 0;

    // Initialize the batcher with configuration
    // @param device The render device
    // @param config Configuration parameters
    virtual void                      Initialize(MIDevice* device, const MTextureBatcherConfig& config) = 0;

    // Release GPU resources
    // @param device The render device
    virtual void                      Release(MIDevice* device) = 0;

    // Register a texture and get its layer index in the texture array
    // Texture will be resized to target size if dimensions differ
    // @param texture The texture to register (must have CPU-accessible data)
    // @return Result containing success status and layer index
    virtual MTextureBatchResult       RegisterTexture(const MTexturePtr& texture) = 0;

    // Unregister a texture by its layer index, freeing the slot for reuse
    // @param texture The texture to register (must have CPU-accessible data)
    virtual void                      UnregisterTexture(const MTexturePtr& texture) = 0;

    // Sync pending texture uploads to GPU
    // Must be called from render thread before using the texture array
    // @param device The render device
    virtual void                      RenderThreadUpdate(MIDevice* device) = 0;

    // Get the combined texture array
    // @return The Texture2DArray containing all registered textures
    [[nodiscard]] virtual MTexturePtr GetTextureArray() const = 0;

    // Get current allocated layer count (capacity)
    [[nodiscard]] virtual uint32_t    GetLayerCount() const = 0;

    // Get number of textures currently registered
    [[nodiscard]] virtual uint32_t    GetUsedCount() const = 0;

    // Get the actual size of texture at given layer index
    // Returns Vector2i(0,0) if index is invalid
    [[nodiscard]] virtual Vector2i    GetTextureSize(int32_t layerIndex) const = 0;
};

}// namespace morty
