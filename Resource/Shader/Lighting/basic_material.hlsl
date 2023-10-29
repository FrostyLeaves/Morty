
struct Material
{
    float3 f3Ambient;
    float fAlphaFactor;
    float3 f3Diffuse;
    int bUseNormalTex;
    float3 f3Specular;
    float fShininess;
    int bUseSpecularTex;
    int bUseTransparentTex;
    int bUseEmissiveTex;
};

//PS
[[vk::binding(0,0)]]cbuffer cbMaterial
{
    Material u_xMaterial;
};

[[vk::binding(1,0)]]Texture2D u_texDiffuse;
[[vk::binding(2,0)]]Texture2D u_texNormal;
[[vk::binding(3,0)]]Texture2D u_texSpecular;
[[vk::binding(4,0)]]Texture2D u_texTransparent;
[[vk::binding(5,0)]]Texture2D u_texEmissive;

