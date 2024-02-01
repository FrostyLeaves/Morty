#include "../PostProcess/post_process_header.hlsl"

//reference: https://zenn.dev/mebiusbox/articles/c7ea4871698ada

#define HBAO_DIRECTION_NUM (4)
#define HBAO_STEP_NUM (4)

[[vk::binding(0,0)]]cbuffer cbHbaoSetting
{
    float u_fNearestAoScale;
    float u_fOtherAoScale;
    float u_fNDotVBias;
    float u_fRadiusSquareNegInv;
}

[[vk::binding(1,0)]]Texture2D u_texInputTexture;  //gbuffer normal
[[vk::binding(2,0)]]Texture2D u_texInputTexture1; //gbuffer depth

[[vk::binding(3, 0)]]cbuffer cbHbaoFrameData
{
    float4 u_f4UVToView;
    float u_fRadiusPixel;
};

struct VS_OUT_POST
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct HbaoPixelData
{
    float3 f3ViewPosition;
    float3 f3ViewNormal;
    float2 f2ScreenUV;
    
    float fViewDepth;
    float fRadiusPixel;
    float fRadiusSquareNegInv;
};

float2 RotateDirection(float2 dir, float2 f2RotationCosSin)
{
    // https://en.wikipedia.org/wiki/Rotation_matrix
    return float2(dir.x * f2RotationCosSin.x - dir.y * f2RotationCosSin.y,
                  dir.x * f2RotationCosSin.y + dir.y * f2RotationCosSin.x);
}

float Falloff(HbaoPixelData data, float fDistanceSquare)
{
    return fDistanceSquare * data.fRadiusSquareNegInv + 1.0f;
}

//f3ViewPosition: pixel position in view space.
//f3ViewNormal: pixel normal in view space.
//f3SamplePosition: pixel position with ray marching in view space.
float ComputeHorizonOcclusion(HbaoPixelData data, float3 f3SamplePosition)
{
    float3 V = f3SamplePosition - data.f3ViewPosition;

    //length * length
    float VdotV = dot(V, V);
    float NdotV = dot(data.f3ViewNormal, V) * rsqrt(VdotV);

    return saturate(NdotV - u_fNDotVBias) * saturate(Falloff(data, VdotV));
}

float PerspectiveDepthToViewDepth(float depth, float near, float far)
{
    float fViewDepth = (near * far) / ( (near - far) * depth + far);

    return fViewDepth;
}

float3 GetViewNormalFromScreenUV(float2 uv)
{
    float3 f3WorldNormal = u_texInputTexture.SampleLevel(NearestSampler, uv, 0).rgb;
    float3 f3ViewNormal = mul(float4(f3WorldNormal, 0.0f), u_matView).rgb;

    return f3ViewNormal;
}

float GetViewDepthFromScreenUV(float2 uv)
{
    float fDepth = u_texInputTexture1.SampleLevel(NearestSampler, uv, 0).r;

    return PerspectiveDepthToViewDepth(fDepth, u_matZNearFar.x, u_matZNearFar.y);
}

float3 GetViewPositionFromScreenUV(float2 uv, float fViewDepth)
{
    float3 f3ViewPosition = float3((u_f4UVToView.xy * uv + u_f4UVToView.zw) * fViewDepth, fViewDepth);

    return f3ViewPosition;
}

float4 GetRandomNoise(HbaoPixelData data)
{
    float2 f2NoiseUV = data.f2ScreenUV * data.fViewDepth / u_matZNearFar.y;
    
    const float4 f4Rand = u_texNoiseTexture.Sample(NearestSampler, f2NoiseUV);
    
    return f4Rand;
}

float ComputeAO(HbaoPixelData data, float fRandomStepOffsetPixel, float2 f2Direction)
{
    float2 f2OffsetPixel = (fRandomStepOffsetPixel * f2Direction);
    float2 f2OffsetUV = f2OffsetPixel * u_f2ViewportSizeInv;

    //TODO: half resolution scale.
    float2 f2SampleUV = data.f2ScreenUV + f2OffsetUV;

    float fSampleViewDepth = GetViewDepthFromScreenUV(f2SampleUV);
    float3 f3SamplePosition = GetViewPositionFromScreenUV(f2SampleUV, fSampleViewDepth);

    return ComputeHorizonOcclusion(data, f3SamplePosition);
}

float HBAO(HbaoPixelData data)
{
    const float fCircleSlice = 2.0f * NUM_PI / HBAO_DIRECTION_NUM;

    const float4 f4Rand = GetRandomNoise(data);
    float2 f2RandDirection = normalize(max(f4Rand.xy, 0.02f));

    const float fStepLengthPixel = data.fRadiusPixel / (HBAO_STEP_NUM + 1);

    float fNearestAoValue = 0.0f;
    float fOtherAoValue = 0.0f;

    for (float nDirectionIdx = 0; nDirectionIdx < HBAO_DIRECTION_NUM; ++nDirectionIdx)
    {
        const float fOriginAngle = fCircleSlice * nDirectionIdx;

        float2 f2RandomDirection = RotateDirection(f2RandDirection, float2(cos(fOriginAngle), sin(fOriginAngle)));

        float fRandomStepOffsetPixel = 1.0f + f4Rand.z * fStepLengthPixel;

        fNearestAoValue += ComputeAO(data, fRandomStepOffsetPixel, f2RandomDirection);

        for (float nStepIdx = 1; nStepIdx < HBAO_STEP_NUM; ++nStepIdx)
        {
            fOtherAoValue += ComputeAO(data, fRandomStepOffsetPixel + fStepLengthPixel * nStepIdx, f2RandomDirection);
        }
    }

    float ao = fNearestAoValue * u_fNearestAoScale + fOtherAoValue * u_fOtherAoScale;

    ao /= (HBAO_DIRECTION_NUM * HBAO_STEP_NUM);

    return ao;
}

float PS_MAIN(VS_OUT_POST input) : SV_Target
{
    float fAOStrength = 2.0f;

    float fViewDepth = GetViewDepthFromScreenUV(input.uv);
    float3 f3ViewNormal = GetViewNormalFromScreenUV(input.uv);

    HbaoPixelData data;
    
    data.f3ViewPosition = GetViewPositionFromScreenUV(input.uv, fViewDepth);
    data.f3ViewNormal = f3ViewNormal;
    data.f2ScreenUV = input.uv;
    data.fViewDepth = fViewDepth;
        
    data.fRadiusPixel = u_fRadiusPixel / data.fViewDepth;
    data.fRadiusSquareNegInv = u_fRadiusSquareNegInv;
    
    float ao = HBAO(data);

    return saturate(1.0 - ao * fAOStrength);;
}