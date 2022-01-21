
[[vk::binding(1,0)]]TextureCube U_SkyBox;
[[vk::binding(2,0)]]sampler LinearSampler;

#define NUM_PI (3.1415926535898)

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 uvw : UVW;
    uint idx : TEST_IDX;
};

float4 PS(VS_OUT input) : SV_Target
{
    float3 f3Normal = normalize(input.uvw);

    float3 f3Irradiance = 0.0;
    
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(f3Normal, up));
    up = normalize(cross(right, f3Normal));
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0;

    for(float phi = 0.0; phi < 2.0 * NUM_PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * NUM_PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * f3Normal; 

            f3Irradiance += U_SkyBox.SampleLevel(LinearSampler, sampleVec.xyz, 0).rgb * cos(theta) * sin(theta);
            nrSamples = nrSamples + 1.0;
        }
    }


    f3Irradiance = NUM_PI * f3Irradiance * (1.0 / nrSamples);
    
    return float4(f3Irradiance, 1.0);
}