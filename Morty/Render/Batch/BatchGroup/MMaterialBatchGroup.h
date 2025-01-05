#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MBuffer.h"
#include "Basic/MStorageVariant.h"
#include "MInstanceBatchGroup.h"
#include "MRenderInstanceCache.h"
#include "Object/MObject.h"
#include "Shader/MShaderParam.h"
#include "Utility/MBounds.h"
#include "Utility/MMemoryPool.h"

namespace morty
{
class MORTY_API MMaterialBatchGroup
{

public:
    void   Initialize(MEngine* pEngine, std::shared_ptr<MMaterialTemplate> pMaterialTemplate);

    void   Release(MEngine* pEngine);

    size_t RegisterMaterial(MMaterialPtr material);


private:
    MEngine*                                                         m_engine              = nullptr;
    std::shared_ptr<MMaterialTemplate>                               m_pMaterialTemplate   = nullptr;
    std::shared_ptr<MShaderPropertyBlock>                            m_shaderPropertyBlock = nullptr;
    std::shared_ptr<MShaderStorageParam>                             m_transformParam      = nullptr;

    std::vector<MemoryInfo>                                          m_transformArray;
    MRenderInstanceCache<MMeshInstanceKey, MMeshInstanceRenderProxy> m_instanceCache;
    MStorageVariant                                                  m_materialBuffer;
};

}// namespace morty
