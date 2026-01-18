module InternalModule;

#include "Internal/PreDefine.slang"
__include Internal.CommonAttributes;
__include Internal.CommonStruct;
__include Internal.CommonUniforms;
__include Internal.CommonInterface;
__include Internal.GPUSceneData;

public struct GlobalRenderData
{
    public FrameData frame;
    public RenderSceneParameters scene;

    public SamplerState LinearSampler;
    public SamplerState NearestSampler;
}

[[vk::binding(0, 1)]] public ParameterBlock<GlobalRenderData> GlobalData;