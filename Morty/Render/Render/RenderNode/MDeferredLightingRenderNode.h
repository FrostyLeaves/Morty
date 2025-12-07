/**
 * @File         MDeferredLightingRenderNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MGBufferRenderNode.h"
#include "Render/RenderGraph/MSinglePassRenderNode.h"

#include "MShadowMapRenderNode.h"
#include "Render/MRenderInfo.h"
#include "Render/RenderGraph/MRenderCommon.h"

namespace morty
{
class MResource;
class MMaterialResource;
REFL_RENDER_NODE_CLASS MDeferredLightingRenderNode : public ISinglePassRenderNode
{
    MORTY_CLASS(MDeferredLightingRenderNode)

public:
    void                      OnCreated() override;
    void                      Release() override;
    void                      Execute(const MRenderInfo& info, IRenderCommand* primaryCommand) override;
    flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder & fbb) override;
    void                      Deserialize(flatbuffers::FlatBufferBuilder & fbb) override;
    void                      Deserialize(const void* flatbuffer) override;

public:
    REFL_RENDER_NODE_PROPERTY bool EnableAO = false;

    REFL_RENDER_NODE_PROPERTY std::shared_ptr<MMaterialResource> LightingMaterial = nullptr;

protected:
    void                               UpdateProperty();

    void                               BindInOutTexture() override;

    std::vector<MRenderTaskInputDesc>  InitInputDesc() override;

    std::vector<MRenderTaskOutputDesc> InitOutputDesc() override;


private:
    std::shared_ptr<MMaterial> m_lightningMaterial = nullptr;
};

}// namespace morty