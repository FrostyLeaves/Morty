
struct Material
{
    float fAlphaFactor;
    float bUseHeightMap;
    
    float4 f4Albedo;
    float fMetallic;
    float fRoughness;
};

//Material
[[vk::binding(0,0)]]cbuffer cbMaterial
{
    Material u_xMaterial;
};

//Textures
[[vk::binding(1,0)]]Texture2D u_mat_texAlbedo;
[[vk::binding(2,0)]]Texture2D u_texNormal;
[[vk::binding(3,0)]]Texture2D u_mat_texMetallic;
[[vk::binding(4,0)]]Texture2D u_mat_texRoughness;
[[vk::binding(5,0)]]Texture2D u_mat_texAmbientOcc;
[[vk::binding(6,0)]]Texture2D u_mat_texHeight;
