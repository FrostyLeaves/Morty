/**
 * @File         MRenderTaskNode
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/MRenderInfo.h"
#include "TaskGraph/MTaskNode.h"
#include "TaskGraph/MTaskNodeInput.h"
#include "Utility/MStringId.h"

namespace morty
{

class MRenderTargetManager;
class MRenderGraph;
class MRenderPass;

struct MRenderTaskInputDesc {
    MStringId             name;
    MRenderNodeOutputType type;
    METextureFormat       format     = METextureFormat::UNorm_RGBA8;
    bool                  allowEmpty = false;
    METextureBarrierStage barrier    = METextureBarrierStage::EPixelShaderSample;

    const MType*          dataType = nullptr;

    MHashCode             GetLinkHash() const;
};

class MORTY_API MRenderTaskNodeInput : public MTaskNodeInput
{
    MORTY_CLASS(MRenderTaskNodeInput)

    void                               SetInputDesc(const MRenderTaskInputDesc& desc);
    [[nodiscard]] MRenderTaskInputDesc GetInputDesc() const { return m_desc; }
    [[nodiscard]] METextureFormat      GetFormat() const { return m_desc.format; }

    template<typename T> [[nodiscard]] const T*        GetData() const { return GetDataInternal()->DynamicCast<T>(); }
    [[nodiscard]] const MTypeClass*                                     GetDataInternal() const;

    static MRenderTaskInputDesc        CreateSample(const MStringId& name, METextureFormat format, bool allowEmpty);
    static MRenderTaskInputDesc        CreatePixelWrite(const MStringId& name, METextureFormat format, bool allowEmpty);
    static MRenderTaskInputDesc        CreateDepth(const MStringId& name);

    template<typename T> static MRenderTaskInputDesc CreateData(const MStringId& name) { return CreateData(name, T::GetClassType()); }
    static MRenderTaskInputDesc CreateData(const MStringId& name, const MType* dataType);

private:
    MRenderTaskInputDesc m_desc;
};

}// namespace morty