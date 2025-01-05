#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"
#include "Resource/MTextureResource.h"

#include "Material/MMaterialTemplate.h"
#include "Shader/MShaderMacro.h"
#include "Shader/MShaderProgram.h"
#include "Shader/MShaderPropertyBlock.h"

namespace morty
{

class MORTY_API MMaterialPropertyProxy
{
public:
    MMaterialPropertyProxy()  = default;
    ~MMaterialPropertyProxy() = default;

    template<typename TYPE> void SetProperty(const MStringId& name, const TYPE& value);
    void                         SetTexture(const MStringId& strName, const std::shared_ptr<MResource>& pTexResource);
    
    void                         BindMaterial(const MMaterialPtr& material);

    flatbuffers::Offset<void>    Serialize(flatbuffers::FlatBufferBuilder& fbb);
    void                         Deserialize(const void* pBufferPointer);

private:
    std::unordered_map<MStringId, MVariant> m_materialProperty;
    size_t                                  m_instanceIdx = 0;
};

}// namespace morty