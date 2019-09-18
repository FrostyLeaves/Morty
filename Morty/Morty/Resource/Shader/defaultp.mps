
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : UV;

    float3 normal : NORMAL;
    float3 worldPos : WORLDPOS;
};

sampler sampler0;
Texture2D texture0;


cbuffer cbLight
{
    float3 AmbientLightColor;
    float3 DiffuseLightPos;
    float3 DiffuseLightColor;

    float3 CameraWorldPos;
};

float4 PS(VS_OUT input) : SV_Target
{
    float4 color = texture0.Sample(sampler0, input.uv);

    // Ambient
    float AmbientStrength = 0.25f;
    float3 ambientColor = AmbientStrength * AmbientLightColor;

    //Diffuse
    float3 norm = normalize(input.normal);
    float3 lightDir = normalize(DiffuseLightPos - input.worldPos);
    float diff = max(dot(lightDir, norm), 0.0f);
    float3 diffuseColor = diff * DiffuseLightColor;

    //Specular
    float specularStrength = 0.5f;

    float3 cameraDir = normalize(CameraWorldPos - input.worldPos);
    float3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(cameraDir, reflectDir), 0.0), 32);
    float3 specularColor = specularStrength * spec * DiffuseLightColor;

    color.xyz = (diffuseColor + ambientColor + specularColor) * color.xyz;

    return color;
}

