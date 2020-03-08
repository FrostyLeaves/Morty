#include "modelHeader.hlsl"

//DirectionLight
float3 CalcDirectionLight(VS_OUT input, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3AmbiColor, float3 f3DiffColor, float3 f3SpecColor)
{
    float fDiff = max(dot(f3Normal, -f3LightDir), 0.0f);

    float3 fReflectDir = reflect(f3LightDir, f3Normal);
    float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), U_mat.fShininess);

    return float3(     U_dirLight.f3Diffuse * U_mat.f3Diffuse * fDiff * f3DiffColor
                     + U_dirLight.f3Specular * U_mat.f3Specular * fSpec * f3SpecColor
                );
}

float3 CalcPointLight(PointLight pointLight, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3WorldPixelPosition, float3 f3AmbiColor, float3 f3DiffColor, float3 f3SpecColor)
{

    float fDiff = max(dot(f3Normal, f3LightDir), 0.0f);

    float3 fReflectDir = reflect(-f3LightDir, f3Normal);
    float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), U_mat.fShininess);

    float fDistance = length(pointLight.f3WorldPosition - f3WorldPixelPosition);
    float fAttenuation = 1.0f / (1.0f + pointLight.fLinear * fDistance + pointLight.fQuadratic * fDistance * fDistance);

    return float3(     pointLight.f3Diffuse * U_mat.f3Diffuse * fDiff * f3DiffColor
                     + pointLight.f3Specular * U_mat.f3Specular * fSpec * f3SpecColor
                ) * fAttenuation;
}

float ShadowCalculation(VS_OUT input, float3 f3Normal, float3 f3DirLightDir)
{
    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (input.dirLightSpacePos.x / input.dirLightSpacePos.w * 0.5f);
    shadowTexCoords.y = 0.5f - (input.dirLightSpacePos.y / input.dirLightSpacePos.w * 0.5f);
    
    if(shadowTexCoords.x < 0.0f || shadowTexCoords.x > 1.0f || shadowTexCoords.y < 0.0f || shadowTexCoords.y > 1.0f)
        return 0.0f;

    float pixelDepth = input.dirLightSpacePos.z / input.dirLightSpacePos.w;

    float NdotL = dot(f3Normal, -f3DirLightDir);
    
    float margin = acos(saturate(NdotL));

    if (NdotL > 0)
    {
        float epsilon = 0.0005 / margin;
        epsilon = clamp(epsilon, 0.0005, 0.001) * 2.0;

        float offset = 1.0f / MSHADOW_TEXTURE_SIZE;
        
        float lighting = 0.0f;

        float fLightTop = 0.0f;
        fLightTop += float(U_texShadowMap.SampleCmpLevelZero(U_shadowMapSampler, shadowTexCoords.xy + float2(-offset, -offset), pixelDepth - epsilon));
        fLightTop += float(U_texShadowMap.SampleCmpLevelZero(U_shadowMapSampler, shadowTexCoords.xy + float2(offset, -offset), pixelDepth - epsilon));
        fLightTop *= 0.5f;

        float fLightBom = 0.0f;
        fLightBom += float(U_texShadowMap.SampleCmpLevelZero(U_shadowMapSampler, shadowTexCoords.xy + float2(-offset, offset), pixelDepth - epsilon));
        fLightBom += float(U_texShadowMap.SampleCmpLevelZero(U_shadowMapSampler, shadowTexCoords.xy + float2(offset, offset), pixelDepth - epsilon));
        fLightBom *= 0.5f;

        lighting = (fLightTop + fLightBom) * 0.5f;

        return lighting;
    }

    return 0.0f;
}

float4 PS(VS_OUT input) : SV_Target
{
    
    float4 f3AmbiColor = U_mat.texDiffuse.Sample(U_defaultSampler, input.uv);
    float4 f3DiffColor = f3AmbiColor;
    float4 f3SpecColor = f3AmbiColor;//U_mat.texSpecular.Sample(U_defaultSampler, input.uv);
    
    float3 f3Normal = float3(0.0f, 0.0f, -1.0f);
    float3 f3CameraDir = float3(0.0f, 0.0f, 1.0f);
    float3 f3DirLightDir = float3(0.0f, 0.0f, 1.0f);


    if (U_mat.bUseNormalTex > 0.5f)
    {
        f3Normal = U_mat.texNormal.Sample(U_defaultSampler, input.uv).xyz;
        f3Normal = f3Normal.rgb * 2.0f - 1.0f;
        f3Normal = normalize(f3Normal);
        
//如果在VS处理Normal
#if MCALC_NORMAL_IN_VS
        //使用法线贴图， 法向量在切线空间， CameraDir也在切线空间
        
        f3CameraDir = input.toCameraDirTangentSpace;
        f3DirLightDir = input.dirLightDirTangentSpace;
#else
        //使用法线贴图，法向量在view space，CameraDir也在view space

        float3 T = normalize(input.tangent);
        float3 B = normalize(input.bitangent);
        float3 N = normalize(input.normal);
        float3x3 TBN = float3x3(T,B,N);

        f3Normal = mul(f3Normal, TBN);
        f3CameraDir = normalize(U_f3CameraPosition - input.worldPos);
        f3DirLightDir = U_f3DirectionLight;
#endif
    }
    else
    {
        //没用法线贴图，法向量在view space，CameraDir也在view space
        f3Normal = input.normal;
        f3CameraDir = normalize(U_f3CameraPosition - input.worldPos);
        f3DirLightDir = U_f3DirectionLight;
    }


    float3 f3Color = U_mat.f3Ambient * f3AmbiColor.xyz * 0.1f;
    float shadow = ShadowCalculation(input, f3Normal, f3DirLightDir);
    
    
    f3Color += shadow * CalcDirectionLight(input, f3CameraDir, f3DirLightDir, f3Normal, f3AmbiColor, f3DiffColor, f3SpecColor);

    //for(int i = 0; i < MPOINT_LIGHT_MAX_NUMBER; ++i)
    {
//如果在VS处理Normal
#if MCALC_NORMAL_IN_VS
        //float3 f3LightDir = input.pointLightDirTangentSpace[i];
#else
        //float3 f3LightDir = normalize(U_pointLights[i].f3WorldPosition - f3WorldPixelPosition);
#endif
        //f3Color += CalcPointLight(U_pointLights[i], f3CameraDir, f3LightDir, f3Normal, input.worldPos, f3AmbiColor, f3DiffColor, f3SpecColor);
    }

    return float4(f3Color, 1.0f);
}

