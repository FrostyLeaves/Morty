implementing TestModule;

public struct TestParameterBlockStruct
{
    public float4 firstParam;
    public float4 secondParam;
};

float4 global_float4_1;
float4 global_float4_2;

public cbuffer global_cbuffer_1
{
    public float4 cbufferParam_1;
    public float4 cbufferParam_2;
}

uniform ParameterBlock<TestParameterBlockStruct> global_block_1;
uniform ParameterBlock<TestParameterBlockStruct> global_block_2;

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
    return global_float4_1 * global_float4_2 * global_block_1.firstParam * global_block_1.secondParam * cbufferParam_1 * cbufferParam_2;
}

[shader("pixel")]
float4 pixelMain2(VertexOut input)
{
    return global_float4_1 * global_float4_2 * global_block_1.firstParam * global_block_1.secondParam * cbufferParam_1 * cbufferParam_2;
}