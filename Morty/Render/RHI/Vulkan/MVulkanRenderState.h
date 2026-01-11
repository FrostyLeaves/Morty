/**
 * @File         MVulkanRenderState
 *
 * @Created      2025-01-11
 *
 * @Author       DoubleYe
 * @Brief        Render state tracking for Vulkan render commands
**/

#pragma once

#include "Utility/MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN

#include <cstdint>
#include <string>

namespace morty
{

// Graphics pipeline binding state tracking
struct MGraphicsPipelineState
{
    bool     pipelineBound       = false;  // Whether a graphics pipeline is bound
    uint32_t requiredSetsMask    = 0;      // Bitmask of descriptor sets required by pipeline
    uint32_t boundSetsMask       = 0;      // Bitmask of descriptor sets currently bound

    void Reset()
    {
        pipelineBound    = false;
        requiredSetsMask = 0;
        boundSetsMask    = 0;
    }

    void SetPipelineBound(uint32_t requiredSets)
    {
        pipelineBound    = true;
        requiredSetsMask = requiredSets;
        boundSetsMask    = 0;  // Reset bound sets when new pipeline is bound
    }

    void SetDescriptorSetBound(uint32_t setIndex)
    {
        boundSetsMask |= (1u << setIndex);
    }

    [[nodiscard]] bool IsValid() const
    {
        if (!pipelineBound) return false;
        // Check if all required sets are bound
        return (boundSetsMask & requiredSetsMask) == requiredSetsMask;
    }

    [[nodiscard]] uint32_t GetMissingSetsMask() const
    {
        return requiredSetsMask & ~boundSetsMask;
    }

    // Get human-readable string of missing set indices, e.g. "0, 2, 3"
    [[nodiscard]] std::string GetMissingSetsString() const
    {
        uint32_t    missingMask = GetMissingSetsMask();
        std::string result;
        for (uint32_t i = 0; i < 32 && missingMask != 0; ++i)
        {
            if (missingMask & (1u << i))
            {
                if (!result.empty()) { result += ", "; }
                result += std::to_string(i);
            }
        }
        return result.empty() ? "none" : result;
    }
};

}// namespace morty

#endif
