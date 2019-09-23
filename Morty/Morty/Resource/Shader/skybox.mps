
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 uvw : UVW;
};

sampler sampler0
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

TextureCube SkyTexCube;

float4 PS(VS_OUT input) : SV_Target
{
    float4 color = SkyTexCube.Sample(sampler0, input.uvw);

  //  color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return color;
}

