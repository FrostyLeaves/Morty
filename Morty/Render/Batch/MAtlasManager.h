#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "MMaterialBatchGroup.h"
#include "Material/MMaterial.h"
#include "Scene/MManager.h"
#include "Variant/MVariant.h"

namespace morty
{

struct MORTY_API MAtlasTextureId {
    size_t atlasId;
    size_t textureId;
};


class MORTY_API MAtlas
{

public:
private:
    MTexturePtr m_atlasTexture = nullptr;
};


class MORTY_API MAtlasManager : public IManager
{
public:
    MORTY_INTERFACE(MAtlasManager)

    void                                             Initialize() override;

    void                                             Release() override;

    void                                             RegisterTexture(const MTexturePtr& texture);

    void                                             UnregisterTexture(const MTexturePtr& texture);

    MAtlasTextureId                                  GetTexture(const MTexturePtr& texture) const;

    std::unordered_map<MTexturePtr, MAtlasTextureId> m_textureTable;

    std::vector<MAtlas>                              m_atlasTable;
};


}// namespace morty