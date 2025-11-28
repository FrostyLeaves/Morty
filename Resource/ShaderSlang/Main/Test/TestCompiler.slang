enum PropertyType: int
{
    int,
    float,
    float3,
    float4,
    color
};

enum ResourceType: int
{
    Texture2D,
    TextureCube
};


[__AttributeUsage(_AttributeTargets.Var)]
struct PropertyAttribute
{
    string displayName;
    PropertyType type;
};

[__AttributeUsage(_AttributeTargets.Var)]
struct ResourceAttribute
{
    string displayName;
    ResourceType type;
};

public struct TestParameterBlockStruct
{
    [Property("Param1", PropertyType.float4)]
    public float4 firstParam;
    [Property("Param2", PropertyType.float4)]
    public float4 secondParam;
};

float4 global_float4_1;
float4 global_float4_2;

public SamplerState LinearSampler;

[Resource("texture1", ResourceType.Texture2D)]
Texture2D<float4> global_texture2D_1;

public cbuffer global_cbuffer_1
{
    public float4 cbufferParam_1;
    public float4 cbufferParam_2;
}

uniform ParameterBlock<TestParameterBlockStruct> global_block_1;

public cbuffer global_cbuffer_2
{
    public float4 cbufferParam_3;
    public float4 cbufferParam_4;
}

struct VertexIn
{
    float3 position : Position;
}

struct VertexOut
{
    float3 position : SV_Position;
}


[shader("vertex")]
VertexOut vertexMain(VertexIn input, uint INSTANCE_ID: SV_InstanceID)
{
    VertexOut output;
    output.position = input.position;

    return output;
}

[shader("pixel")]
float4 pixelMain(VertexOut input)
{
    float4 uv = global_float4_1 * global_float4_2 * global_block_1.firstParam * global_block_1.secondParam * cbufferParam_1 * cbufferParam_2;
    return global_texture2D_1.Sample(LinearSampler, uv.xy);
}

[shader("pixel")]
float4 pixelMain2(VertexOut input)
{
    return global_float4_1 * global_float4_2 * global_block_1.firstParam * global_block_1.secondParam * cbufferParam_1 * cbufferParam_2;
}