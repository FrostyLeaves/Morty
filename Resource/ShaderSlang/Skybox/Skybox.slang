implementing SkyboxModule;

public struct SkyboxData
{
    public float intensity;
    public float rotation;
    public float3 tint;
}

public struct Skybox
{
    public TextureCube skyboxTexture;
    
    public float3 Sample(float3 direction, SkyboxData data)
    {
        // Apply rotation
        float cosTheta = cos(data.rotation);
        float sinTheta = sin(data.rotation);
        float3 rotatedDir = float3(
            direction.x * cosTheta - direction.z * sinTheta,
            direction.y,
            direction.x * sinTheta + direction.z * cosTheta
        );
        
        float3 color = skyboxTexture.Sample(LinearSampler, rotatedDir).rgb;
        return color * data.intensity * data.tint;
    }
}

public struct SkyboxVertex
{
    float3 pos : POSITION;
}

public struct SkyboxPixel
{
    float4 pos : SV_POSITION;
    float3 localPos : TEXCOORD;
}
