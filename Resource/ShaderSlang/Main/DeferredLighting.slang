/*
    延迟渲染光照计算阶段
*/

import InternalModule;
import Deferred.DeferredModule;
import Lighting.LightingModule;

// GBuffer input
Texture2D<float4> gBuffer0;
Texture2D<float4> gBuffer1;
Texture2D<float4> gBuffer2;
Texture2D<float> depthTexture;

// IBL input
public TextureCube irradianceMap;
public TextureCube prefilterMap;
public Texture2D brdfLUT;


// 用于从深度重建位置的矩阵
ParameterBlock<ReconstructionData> reconstructionData;

struct ReconstructionData
{
    float4x4 invViewProjMatrix;
}

struct VertexIn
{
    float2 pos : POSITION;
}

struct VertexOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
}

struct PixelOut
{
    float4 target : SV_Target;
}

[shader("vertex")]
VertexOut VS_MAIN(VertexIn input, uint INSTANCE_ID: SV_InstanceID)
{
    VertexOut output;

    output.pos.xy = input.pos.xy;
    output.pos.w = 1.0f;
    output.pos.z = 0.0f;
    output.uv.x = output.pos.x * 0.5f + 0.5f;
    output.uv.y = 0.5f - output.pos.y * 0.5f;

    return output;
}


[shader("pixel")]
PixelOut PS_MAIN(VertexOut input)
{
    PixelOut output;

    GBufferLoader gBufferLoader;
    gBufferLoader.gBuffer0 = gBuffer0;
    gBufferLoader.gBuffer1 = gBuffer1;
    gBufferLoader.gBuffer2 = gBuffer2;
    gBufferLoader.depthTexture = depthTexture;
    gBufferLoader.invViewProjMatrix = reconstructionData.invViewProjMatrix;

    FrameData frameData = GlobalFrameData;

    var surfaceData = gBufferLoader.LoadGBuffer(input.uv);
    surfaceData.cameraDir = normalize(frameData.mainCamera.positionWS - surfaceData.positionWS);

    LightingSystem lightingSystem;
    LightingOutput lightingOutput = lightingSystem.ComputeLighting(surfaceData);


    // IBL环境光
    IBLSystem iblSystem;

    IBLData iblData;
    iblData.intensity = 1.0;
    iblData.maxReflectionLod = 4.0;
    iblData.irradianceMap = irradianceMap;
    iblData.prefilterMap = prefilterMap;
    iblData.brdfLUT = brdfLUT;

    float3 ambient = iblSystem.CalculateIBL(surfaceData, iblData);


    // 最终颜色合成
    float3 finalColor = lightingOutput.color + ambient;

    output.target = float4(finalColor, 1.0f);
    return output;
}