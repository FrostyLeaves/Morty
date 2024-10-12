#pragma once

#include <memory>
#include <stdint.h>

#define ImDrawIdx uint32_t

//#define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT \
//struct ImDrawVert{ImVec2 pos;ImVec2 uv;ImColor col;};

namespace morty
{
class MTexture;
}

struct ImGuiTexture {
    std::shared_ptr<morty::MTexture> pTexture    = nullptr;
    intptr_t                         nTextureIdx = 0;
    size_t                           nArrayIdx   = 0;

    ImGuiTexture() = default;

    operator intptr_t() const { return nTextureIdx; }

    ImGuiTexture(void* tex)
        : nTextureIdx(intptr_t(tex))
    {}

    ImGuiTexture(
            std::shared_ptr<morty::MTexture> tex,
            intptr_t                         nTextureIdx,
            size_t                           arrIdx
    )
        : pTexture(tex)
        , nTextureIdx(nTextureIdx)
        , nArrayIdx(arrIdx)
    {}

    bool operator==(const ImGuiTexture& other) const
    {
        return pTexture == other.pTexture && nArrayIdx == other.nArrayIdx;
    }

    bool operator<(const ImGuiTexture& other) const
    {
        if (pTexture < other.pTexture) return true;
        if (pTexture > other.pTexture) return false;

        return nArrayIdx < other.nArrayIdx;
    }

    bool operator!=(const ImGuiTexture& other) { return !operator==(other); }
};

#define ImTextureID ImGuiTexture