implementing DeferredModule;

public struct GBufferPixelData
{
    public float4 albedoMetalness;  // RGB: Albedo, A: Metalness
    public float4 normalRoughness;  // RGB: Normal, A: Roughness
    public float ao;                      // R: AO, G: unused (可用于其他数据)。
};