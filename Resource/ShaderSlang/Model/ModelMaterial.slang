implementing ModelModule;

public struct MaterialData
{
    [Property("albedo", PropertyType.color)]
    float3 albedo;
    [Property("metalness", PropertyType.float)]
    float metalness;
    [Property("roughness", PropertyType.float)]
    float roughness;
}

public struct TextureBatchData
{
    int texAlbedoIndex;
    int texNormalIndex;
    int texMetallicIndex;
    int texRoughnessIndex;
    int texAOIndex;
    int texHeightIndex;
}

public struct MaterialBatchData
{

    [PerInstance(InstanceType.Property)] StructuredBuffer<MaterialData> propertyBatch;
    [PerInstance(InstanceType.TextureIndex)] StructuredBuffer<TextureBatchData> textureIndexBatch;

    [Resource("texAlbedo", ResourceType.Texture2D)]
    Texture2DArray albedoTexture;
    [Resource("texNormal", ResourceType.Texture2D)]
    Texture2DArray normalTexture;
    [Resource("texMetallic", ResourceType.Texture2D)]
    Texture2DArray metallicTexture;
    [Resource("texRoughness", ResourceType.Texture2D)]
    Texture2DArray roughnessTexture;
    [Resource("texAO", ResourceType.Texture2D)]
    Texture2DArray aoTexture;
    [Resource("texHeight", ResourceType.Texture2D)]
    Texture2DArray heightTexture;
}

public struct ModelMaterial : IMaterial
{
    public MaterialBatchData materialDataBatch;

    public SurfaceData GetSurfaceData(PixelData pixelData)
    {
        SurfaceData data;
        MaterialData materialData = materialDataBatch.propertyBatch[pixelData.instanceId];
        TextureBatchData textureIndexBatch = materialDataBatch.textureIndexBatch[pixelData.instanceId];

        float3 normal = materialDataBatch.normalTexture.Sample(GlobalData.LinearSampler, float3(pixelData.uv, textureIndexBatch.texNormalIndex)).xyz;
        normal = (normal * 2.0f) - 1.0f;
        normal = mul(normal, pixelData.TBN);
        normal = normalize(normal);

        float3 texAlbedo = materialDataBatch.albedoTexture.Sample(GlobalData.LinearSampler, float3(pixelData.uv, textureIndexBatch.texAlbedoIndex)).rgb;
        float texMetalness = materialDataBatch.metallicTexture.Sample(GlobalData.LinearSampler, float3(pixelData.uv, textureIndexBatch.texMetallicIndex)).r;
        float texRoughness = materialDataBatch.roughnessTexture.Sample(GlobalData.LinearSampler, float3(pixelData.uv, textureIndexBatch.texRoughnessIndex)).r;
        float texAmbientOcc = materialDataBatch.aoTexture.Sample(GlobalData.LinearSampler, float3(pixelData.uv, textureIndexBatch.texAOIndex)).r;

        data.albedo = texAlbedo * materialData.albedo;
        data.metalness = texMetalness * materialData.metalness;
        data.normalWS = normal;
        data.roughness = texRoughness * materialData.roughness;
        data.positionWS = pixelData.positionWS;
        data.ao = texAmbientOcc;

        return data;
    }
}