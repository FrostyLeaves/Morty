implementing CullingModule;

// BuildDrawCall parameters (using ParameterBlock to avoid push constants)
public struct BuildDrawCallParams
{
    public uint drawGroupCount;
};

[[vk::binding(0, 0)]]
public ParameterBlock<BuildDrawCallParams> buildDrawCallParams;
