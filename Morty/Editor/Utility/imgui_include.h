#pragma once

#include <stdint.h>

#define ImDrawIdx uint32_t

#define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT \
struct ImDrawVert{ImVec2 pos;ImVec2 uv;ImColor col;};


class MTexture;
struct ImGuiTexture
{
    MTexture* pTexture = nullptr;
    size_t nArrayIdx = 0;

    ImGuiTexture() : pTexture(nullptr), nArrayIdx(0) {}

    operator intptr_t() const { return (intptr_t)pTexture; }

    ImGuiTexture(void* tex) : pTexture((MTexture*)tex), nArrayIdx(0) {}
    
    ImGuiTexture(class MTexture* tex, size_t arrIdx) : pTexture(tex), nArrayIdx(arrIdx) {}

    bool operator==(const ImGuiTexture& other)
    {
        return pTexture == other.pTexture
            && nArrayIdx == other.nArrayIdx;
    }

    bool operator<(const ImGuiTexture& other) const
    {
        if (pTexture < other.pTexture)
            return true;
        if (pTexture > other.pTexture)
            return false;

        return nArrayIdx < other.nArrayIdx;
    }

    bool operator!=(const ImGuiTexture& other)
    {
        return !operator==(other);
    }
};

#define ImTextureID ImGuiTexture