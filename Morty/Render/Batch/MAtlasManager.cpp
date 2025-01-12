#include "MAtlasManager.h"

using namespace morty;


constexpr uint32_t MinAtlasTextureSize = 128;


void               MAtlasManager::RegisterTexture(const MTexturePtr& texture)
{
    if (nullptr == texture) { return; }

    const uint32_t texSize = std::max(texture->GetSize2D().x, texture->GetSize2D().y);

    const size_t   atlasId = static_cast<size_t>(std::ceil(std::log2(texSize / MinAtlasTextureSize)));

    MAtlas&        atlas = m_atlasTable[atlasId];
}

void            MAtlasManager::UnregisterTexture(const MTexturePtr& texture) {}

MAtlasTextureId MAtlasManager::GetTexture(const MTexturePtr& texture) const {}