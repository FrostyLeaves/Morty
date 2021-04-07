#include "inner_constant.hlsl"
#include "inner_functional.hlsl"


//VS    per mesh
[[vk::binding(0,2)]]cbuffer _M_E_cbMeshMatrix : register(b0)
{
    float4x4 U_matWorld;
    float3x3 U_matNormal;
};

//VS    with bones
[[vk::binding(0,3)]]cbuffer _M_E_cbAnimation : register(b2)
{
    float4x4 U_vBonesMatrix[128];
};


////////////////////////////////////////////////////////////////

struct LightBasicInfo
{
    float3 f3Normal;
    float3 f3CameraDir;
    float3 f3DirLightDir;
};
