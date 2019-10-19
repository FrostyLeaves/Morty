struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : UV;

    float3 normal : NORMAL;
    float3 worldPos : WORLDPOS;
};

struct Material
{
    Texture2D texDiffuse;
    Texture2D texSpecular;
    float fShininess;
};

struct DirectionLight
{
    float3 f3Direction;
    float3 f3Ambient;
    float3 f3Diffuse;
    float3 f3Specular;
};

struct PointLight
{
    float3 f3WorldPosition;

    float3 f3Ambient;
    float3 f3Diffuse;
    float3 f3Specular;

    float fConstant;
    float fLinear;
    float fQuadratic;

};

sampler U_defaultSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
    AddressW = Wrap;
};

cbuffer cbMaterial
{
    Material U_mat;
};

cbuffer cbLights
{
    DirectionLight U_dirLight;
    PointLight U_pointLights[4];
};

cbuffer cbWorldInfo
{
    float3 U_f3CameraWorldPos;
};

//DirectionLight
float3 CalcDirectionLight(DirectionLight dirLight, float3 f3CameraDir, float3 f3Normal, float3 f3DiffColor, float3 f3SpecColor)
{
    float3 f3LightDir = normalize(-dirLight.f3Direction);
    
    float fDiff = max(dot(f3LightDir, f3Normal), 0.0f);

    float3 fReflectDir = reflect(-f3LightDir, f3Normal);
    float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), U_mat.fShininess);

    return float3(  dirLight.f3Ambient * f3DiffColor + 
                    dirLight.f3Diffuse * fDiff * f3DiffColor + 
                    dirLight.f3Specular * fSpec * f3SpecColor
                );

}

float3 CalcPointLight(PointLight pointLight, float3 f3CameraDir, float3 f3Normal, float3 f3WorldPixelPoint, float3 f3DiffColor, float3 f3SpecColor)
{
    float3 f3LightDir = normalize(pointLight.f3WorldPosition - f3WorldPixelPoint);

    float fDiff = max(dot(f3Normal, f3LightDir), 0.0f);

    float3 fReflectDir = reflect(-f3LightDir, f3Normal);
    float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), U_mat.fShininess);

    float fDistance = length(pointLight.f3WorldPosition - f3WorldPixelPoint);
    float fAttenuation = 1.0f / (pointLight.fConstant + pointLight.fLinear * fDistance + pointLight.fQuadratic * fDistance * fDistance);

    return float3(  pointLight.f3Ambient * f3DiffColor + 
                    pointLight.f3Diffuse * fDiff * f3DiffColor + 
                    pointLight.f3Specular * fSpec * f3SpecColor
                ) * fAttenuation;
}

float4 PS(VS_OUT input) : SV_Target
{
    float3 f3Color = float3(0.0f, 0.0f, 0.0f);
    float4 f3DiffColor = U_mat.texDiffuse.Sample(U_defaultSampler, input.uv);
    float4 f3SpecColor = U_mat.texSpecular.Sample(U_defaultSampler, input.uv);
    float3 f3CameraDir = normalize(U_f3CameraWorldPos - input.worldPos);
    float3 f3Normal = normalize(input.normal);

    f3Color += CalcDirectionLight(U_dirLight, f3CameraDir, f3Normal, f3DiffColor, f3SpecColor);
    f3Color += CalcPointLight(U_pointLights[0], f3CameraDir, f3Normal, input.worldPos, f3DiffColor, f3SpecColor);
    f3Color += CalcPointLight(U_pointLights[1], f3CameraDir, f3Normal, input.worldPos, f3DiffColor, f3SpecColor);
    f3Color += CalcPointLight(U_pointLights[2], f3CameraDir, f3Normal, input.worldPos, f3DiffColor, f3SpecColor);
    f3Color += CalcPointLight(U_pointLights[3], f3CameraDir, f3Normal, input.worldPos, f3DiffColor, f3SpecColor);
    
    return float4(f3Color, 1.0f);
}

