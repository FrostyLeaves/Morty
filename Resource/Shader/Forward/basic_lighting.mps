#include "../Internal/internal_uniform_global.hlsl"
#include "../Internal/internal_functional.hlsl"
#include "../Deferred/pbr_uniform_material.hlsl"
#include "../Model/universal_vsout.hlsl"
#include "../Lighting/pbr_lighting.hlsl"


[[vk::binding(3,0)]]Texture2DArray u_texShadowMap;
[[vk::binding(4,0)]]Texture2D u_mat_SSAO;

//Transparent
[[vk::input_attachment_index(0)]] [[vk::binding(0, 3)]] SubpassInput u_texSubpassInput0;
[[vk::input_attachment_index(1)]] [[vk::binding(1, 3)]] SubpassInput u_texSubpassInput1;


struct PS_OUT
{
    float4 f4FrontColor: SV_Target0;
    float4 fBackColor: SV_Target1;
    float fFrontDepth: SV_Target2;
    float fBackDepth: SV_Target3;
};


float3 GetPixelColor(VS_OUT input)
{
    float2 uv = input.uv;
    uv = saturate(uv);

    float3 T = normalize(input.tangent);
    float3 B = normalize(input.bitangent);
    float3 N = normalize(input.normal);
    float3x3 TBN = float3x3(T,B,N);


    float3 f3Normal = float3(0.0f, 0.0f, 1.0f);
    f3Normal = u_mat_texNormal.Sample(LinearSampler, uv).xyz;
    f3Normal = (f3Normal * 2.0f) - 1.0f;
    f3Normal = mul(f3Normal, TBN);
    f3Normal = normalize(f3Normal);

    float3 f3Albedo   = u_mat_texAlbedo.Sample(LinearSampler, uv).rgb;
    float fMetallic   = u_mat_texMetallic.Sample(LinearSampler, uv).r;
    float fRoughness  = u_mat_texRoughness.Sample(LinearSampler, uv).r;
    float fAmbientOcc = u_mat_texAmbientOcc.Sample(LinearSampler, uv).r;
    float fSSAO = u_mat_SSAO.Sample(NearestSampler, input.uv).x;

    float3 f3WorldPosition = input.worldPos;
    float3 f3CameraDir = normalize(u_f3CameraPosition - f3WorldPosition);
    float fAO = fAmbientOcc * fSSAO;
    
    
    SurfaceData pointData;
    pointData.f3CameraDir = f3CameraDir;
    pointData.f3Normal = f3Normal;
    pointData.f3WorldPosition = f3WorldPosition;
    pointData.f3Albedo = f3Albedo;
    pointData.fRoughness = fRoughness;
    pointData.fMetallic = fMetallic;
    pointData.bReceiveShadow = true;

    float3 f3LightColor = PbrLighting(pointData, u_texShadowMap);

    float3 f3Ambient = Ambient(pointData);

    float4 f4VXGIColor = float4(0,0,0,0);

    float3 f3Color = (f3LightColor + f4VXGIColor.rgb + f3Ambient) * fAO;

    return f3Color;
}


PS_OUT PS_MAIN(VS_OUT input)
{
    PS_OUT output;

    float3 f3Color = GetPixelColor(input);
    float fAlpha = saturate(u_xMaterial.fAlphaFactor);

    
    float fZDepth = input.pos.z;
    float fZFront = u_texSubpassInput0.SubpassLoad().r;
    float fZBack = u_texSubpassInput1.SubpassLoad().r;

    output.f4FrontColor = float4(0, 0, 0, 0);
    output.fBackColor = float4(0, 0, 0, 0);
    output.fFrontDepth = 1;
    output.fBackDepth = 0;

    // a <= b
    clip(fZDepth + NUM_BIAS - fZFront);
    clip(fZBack + NUM_BIAS - fZDepth);

    if(fZDepth - NUM_BIAS > fZFront && fZDepth + NUM_BIAS < fZBack)
    {
        output.fFrontDepth = input.pos.z;
        output.fBackDepth = input.pos.z;
        return output;
    }

    // color = destColor + srcColor * srcAlpha * (1 - destAlpha)
    // return [srcColor * srcAlpha] as srcColor
    // blend destColor * 1 + srcColor * (1 - destAlpha)
    if(fZFront - NUM_BIAS <= fZDepth && fZDepth <= fZFront + NUM_BIAS)
        output.f4FrontColor = float4(f3Color * fAlpha, fAlpha);
    else
        output.fBackColor = float4(f3Color, fAlpha);
    
    return output;
}
