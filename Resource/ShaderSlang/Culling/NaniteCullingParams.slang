implementing CullingModule;

// Nanite culling parameters (using ParameterBlock to avoid push constants)
public struct NaniteCullingParams
{
    public NaniteRenderData renderData;
    public uint instanceCount;
};

[[vk::binding(0, 0)]]
public ParameterBlock<NaniteCullingParams> naniteCullingParams;
