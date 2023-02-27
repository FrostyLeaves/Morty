
[[vk::binding(1,0)]]TextureCube u_texSkyBox;
[[vk::binding(2,0)]]sampler LinearSampler;

#ifndef NUM_PI
    #define NUM_PI (3.1415926535898)
#endif

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 uvw : UVW;
    uint idx : TEST_IDX;
};

float4 PS_MAIN(VS_OUT input) : SV_Target
{
    float3 f3Normal = normalize(input.uvw);

    float3 f3Irradiance = 0.0;
    
    float3 f3UpDirection = float3(0.0, 1.0, 0.0);
    float3 f3RightDirection = normalize(cross(f3Normal, f3UpDirection));
    f3UpDirection = normalize(cross(f3RightDirection, f3Normal));
       
    float fSampleDelta = 0.025;
    float fNrSamples = 0.0;

    for(float phi = 0.0; phi < 2.0 * NUM_PI; phi += fSampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * NUM_PI; theta += fSampleDelta)
        {
            // spherical to cartesian (in tangent space)
            float3 f3TangentSample = float3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 f3SampleDirection = f3TangentSample.x * f3RightDirection + f3TangentSample.y * f3UpDirection + f3TangentSample.z * f3Normal; 

            f3Irradiance += u_texSkyBox.SampleLevel(LinearSampler, f3SampleDirection, 0).rgb * cos(theta) * sin(theta);
            fNrSamples = fNrSamples + 1.0;
        }
    }

    f3Irradiance = NUM_PI * f3Irradiance * (1.0 / fNrSamples);
    return float4(f3Irradiance, 1.0);
}