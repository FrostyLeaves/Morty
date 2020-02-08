#include "modelHeader.hlsl"


//DirectionLight
float3 CalcDirectionLight(DirectionLight dirLight, float3 f3CameraDir, float3 f3Normal, float3 f3AmbiColor, float3 f3DiffColor, float3 f3SpecColor)
{
    float3 f3LightDir = normalize(-dirLight.f3Direction);
    
    float fDiff = max(dot(f3Normal, f3LightDir), 0.0f);

    float3 fReflectDir = reflect(-f3LightDir, f3Normal);
    float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), U_mat.fShininess);

    return float3(  //dirLight.f3Ambient * U_mat.f3Ambient * f3AmbiColor + 
                    dirLight.f3Diffuse * U_mat.f3Diffuse * fDiff * f3DiffColor + 
                    dirLight.f3Specular * U_mat.f3Specular * fSpec * f3SpecColor
                );

}

float3 CalcPointLight(PointLight pointLight, float3 f3CameraDir, float3 f3Normal, float3 f3WorldPixelPosition, float3 f3AmbiColor, float3 f3DiffColor, float3 f3SpecColor)
{
    float3 f3LightDir = normalize(pointLight.f3WorldPosition - f3WorldPixelPosition);

    float fDiff = max(dot(f3Normal, f3LightDir), 0.0f);

    float3 fReflectDir = reflect(-f3LightDir, f3Normal);
    float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), U_mat.fShininess);

    float fDistance = length(pointLight.f3WorldPosition - f3WorldPixelPosition);
    float fAttenuation = 1.0f / (1.0f + pointLight.fLinear * fDistance + pointLight.fQuadratic * fDistance * fDistance);

    return float3(  //pointLight.f3Ambient * U_mat.f3Ambient * f3AmbiColor + 
                    pointLight.f3Diffuse * U_mat.f3Diffuse * fDiff * f3DiffColor + 
                    pointLight.f3Specular * U_mat.f3Specular * fSpec * f3SpecColor
                ) * fAttenuation;
}

float ShadowCalculation(VS_OUT input)
{
    float3 f3Normal = normalize(input.normal);
    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (input.lightSpacePos.x / input.lightSpacePos.w * 0.5f);
    shadowTexCoords.y = 0.5f - (input.lightSpacePos.y / input.lightSpacePos.w * 0.5f);
    float pixelDepth = input.lightSpacePos.z / input.lightSpacePos.w;

    float lighting = 1;

    float3 f3LightDir = normalize(-U_dirLight.f3Direction);
    float NdotL = dot(f3Normal, f3LightDir);
    float margin = acos(saturate(NdotL));

    if (NdotL > 0)
    {
        float epsilon = 0.0005 / margin;
        epsilon = clamp(epsilon, 0.005, 0.01) * 2.0;

        lighting = float(U_mat.texShadowMap.SampleCmpLevelZero(U_shadowMapSampler, shadowTexCoords.xy, pixelDepth - epsilon));

        return lighting;
    }

    return 1.0;
}

float4 PS(VS_OUT input) : SV_Target
{
    
    float4 f3AmbiColor = U_mat.texDiffuse.Sample(U_defaultSampler, input.uv);
    float4 f3DiffColor = f3AmbiColor;
    float4 f3SpecColor = U_mat.texSpecular.Sample(U_defaultSampler, input.uv);
    float3 f3CameraDir = normalize(U_f3CameraWorldPos - input.worldPos);
    float3 f3Normal = normalize(input.normal);

    
    float3 f3Color = f3AmbiColor * 0.1f;
    float shadow = ShadowCalculation(input);
    

    f3Color += shadow * CalcDirectionLight(U_dirLight, f3CameraDir, f3Normal, f3AmbiColor, f3DiffColor, f3SpecColor);
    f3Color += shadow * CalcPointLight(U_pointLights[0], f3CameraDir, f3Normal, input.worldPos, f3AmbiColor, f3DiffColor, f3SpecColor);
    
    return float4(f3Color, 1.0f);
}

