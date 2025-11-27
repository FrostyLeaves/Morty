implementing ModelModule;

internal struct MaterialData
{
    [Property("albedo", PropertyType.color)]
    float3 albedo;

    [Property("metalness", PropertyType.float)]
    float metalness;

    [Property("roughness", PropertyType.float)]
    float roughness;
}

[[vk::binding(0, 0)]] ParameterBlock<MaterialData> materialData;

[Resource("texAlbedo", ResourceType.Texture2D)]
[[vk::binding(1, 0)]] Texture2D albedoTexture;
[Resource("texNormal", ResourceType.Texture2D)]
[[vk::binding(2, 0)]] Texture2D normalTexture;
[[Resource("texMetallic", ResourceType.Texture2D)]]
[[vk::binding(3, 0)]] Texture2D metallicTexture;
[Resource("texRoughness", ResourceType.Texture2D)]
[[vk::binding(4, 0)]] Texture2D roughnessTexture;
[Resource("texAO", ResourceType.Texture2D)]
[[vk::binding(5, 0)]] Texture2D aoTexture;
[Resource("texHeight", ResourceType.Texture2D)]
[[vk::binding(6, 0)]] Texture2D heightTexture;

public struct ModelMaterial : IMaterial
{
    public SurfaceData GetSurfaceData(PixelData pixelData)
    {
        SurfaceData data;

        float3 normal = normalTexture.Sample(LinearSampler, pixelData.uv).xyz;
        normal = (normal * 2.0f) - 1.0f;
        normal = mul(normal, pixelData.TBN);
        normal = normalize(normal);

        float3 texAlbedo = albedoTexture.Sample(LinearSampler, pixelData.uv).rgb;
        float texMetalness = metallicTexture.Sample(LinearSampler, pixelData.uv).r;
        float texRoughness = roughnessTexture.Sample(LinearSampler, pixelData.uv).r;
        float texAmbientOcc = aoTexture.Sample(LinearSampler, pixelData.uv).r;

        data.albedo = texAlbedo * materialData.albedo;
        data.metalness = texMetalness * materialData.metalness;
        data.normalWS = normal;
        data.roughness = texRoughness * materialData.roughness;
        data.positionWS = pixelData.positionWS;
        data.ao = texAmbientOcc;

        return data;
    }
}