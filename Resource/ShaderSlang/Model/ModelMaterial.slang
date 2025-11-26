implementing ModelModule;

internal struct MaterialData
{
    [Property("Albedo", Color)]
    float3 albedo;

    [Property("Metalness", Slider, 0.0, 1.0)]
    float metalness;

    [Property("Roughness", Slider, 0.0, 1.0)]
    float roughness;
}

[[vk::binding(0, 0)]] ParameterBlock<MaterialData> materialData;

[[vk::binding(1, 0)]] Texture2D albedoTexture;
[[vk::binding(2, 0)]] Texture2D normalTexture;
[[vk::binding(3, 0)]] Texture2D metallicTexture;
[[vk::binding(4, 0)]] Texture2D roughnessTexture;
[[vk::binding(5, 0)]] Texture2D aoTexture;
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