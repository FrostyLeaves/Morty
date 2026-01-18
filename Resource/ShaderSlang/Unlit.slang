import InternalModule;
import Model.ModelModule;

public struct UnlitMaterialData
{
    public float4 baseColor;
    public float alphaClip;
}

[[vk::binding(1, 0)]] Texture2D baseColorTexture;
ParameterBlock<UnlitMaterialData> unlitMaterialData;

public struct UnlitVertexIn
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
}

public struct UnlitVertexOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
}

public struct UnlitPixelOut
{
    float4 color : SV_Target0;
}

[shader("vertex")]
UnlitVertexOut VS_MAIN(UnlitVertexIn input, uint INSTANCE_ID: SV_InstanceID)
{
    FrameData frameData = GlobalData.frame;
    StorageBatchMeshData model;
    MeshInstanceRenderProxy instanceProxy = model.GetInstanceProxy(INSTANCE_ID);
    
    float4 positionWS = mul(float4(input.pos, 1.0), instanceProxy.matWorld);
    
    UnlitVertexOut output;
    output.pos = mul(positionWS, frameData.mainCamera.viewMatrix);
    output.uv = input.uv;
    
    return output;
}

[shader("pixel")]
UnlitPixelOut PS_MAIN(UnlitVertexOut input)
{
    float4 texColor = baseColorTexture.Sample(GlobalData.LinearSampler, input.uv);
    float4 finalColor = texColor * unlitMaterialData.baseColor;
    
    // Alpha clipping
    if (finalColor.a < unlitMaterialData.alphaClip)
    {
        discard;
    }
    
    UnlitPixelOut output;
    output.color = finalColor;
    return output;
}
