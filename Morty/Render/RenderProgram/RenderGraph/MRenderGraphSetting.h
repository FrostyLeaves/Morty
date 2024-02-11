/**
 * @File         MRenderGraphSetting
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       DoubleYe
**/

#pragma once


#include "Basic/MTexture.h"
#include "Render/MRenderPass.h"
#include "TaskGraph/MTaskNodeOutput.h"
#include "Utility/MGlobal.h"

#include "RenderProgram/MRenderInfo.h"
#include "RenderProgram/MeshRender/MIndirectIndexRenderable.h"
#include "TaskGraph/MTaskGraph.h"
#include "RenderProgram/RenderGraph/MRenderTargetManager.h"
#include "Utility/MStringId.h"

class IPropertyBlockAdapter;
class MRenderTaskTarget;
class MRenderTargetManager;

class MORTY_API MRenderGraphSetting
{
public:

    template<typename TYPE>
    void RegisterProperty(const MStringId& name, const TYPE& value);
    std::unordered_map<MStringId, MVariant>& GetSettings() { return m_tSettings; }

    template<typename TYPE>
    TYPE GetValue(const MStringId& name) const;

    void MarkDirty(const MStringId& name);
    bool IsDirty(const MStringId& name) const;
    void FlushDirty();

    void RegisterPropertyVariant(const MStringId& name, const MVariant& value);
    MVariant GetPropertyVariant(const MStringId& name) const;

private:

    std::unordered_map<MStringId, MVariant> m_tSettings;
    std::unordered_set<MStringId> m_tDirty;
};

template <typename TYPE>
inline void MRenderGraphSetting::RegisterProperty(const MStringId& name, const TYPE& value)
{
    RegisterPropertyVariant(name, MVariant(value));
}

template<typename TYPE>
inline TYPE MRenderGraphSetting::GetValue(const MStringId& name) const
{
    return GetPropertyVariant(name).GetValue<TYPE>();
}
