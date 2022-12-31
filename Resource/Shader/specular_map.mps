#include "brdf_functional.hlsl"

[[vk::binding(1,0)]]TextureCube u_texSkyBox;
[[vk::binding(2,0)]]sampler LinearSampler;

[[vk::binding(3,0)]]cbuffer cbParam
{
    float u_roughness;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 uvw : UVW;
    uint idx : TEST_IDX;
};

float4 PS_MAIN(VS_OUT input) : SV_Target
{
    float3 f3Normal = normalize(input.uvw);

    // make the simplyfying assumption that V equals R equals the normal 
    float3 R = f3Normal;
    float3 V = R;

    const int SAMPLE_COUNT = 1024;
    float3 f3PrefilteredColor = float3(0.0, 0.0, 0.0);
    float totalWeight = 0.0;
    
    for(int i = 0; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, f3Normal, u_roughness);
        float3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(f3Normal, L), 0.0);
        if(NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D   = DistributionGGX(f3Normal, H, u_roughness);
            float NdotH = max(dot(f3Normal, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * NUM_PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = u_roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            
            f3PrefilteredColor += u_texSkyBox.SampleLevel(LinearSampler, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }

    f3PrefilteredColor = f3PrefilteredColor / totalWeight;

    return float4(f3PrefilteredColor, 1.0);
}