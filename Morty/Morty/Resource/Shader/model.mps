#include "modelHeader.hlsl"


float ShadowCalculation(float4 dirLightSpacePos, float fNdotL)
{
    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (dirLightSpacePos.x / dirLightSpacePos.w * 0.5f);
    shadowTexCoords.y = 0.5f - (dirLightSpacePos.y / dirLightSpacePos.w * 0.5f);
    
	if (saturate(shadowTexCoords.x) == shadowTexCoords.x && saturate(shadowTexCoords.y) == shadowTexCoords.y)
    {     
        float margin = acos(saturate(fNdotL));
        float epsilon = clamp(margin / 1.570796, 0.001f, 0.003f);

        float lighting = 0.0f;
        
        float pixelDepth = dirLightSpacePos.z / dirLightSpacePos.w - epsilon;
        float offset = 1.0f / MSHADOW_TEXTURE_SIZE;
        lighting += float(U_texShadowMap.SampleCmpLevelZero(U_shadowMapSampler, shadowTexCoords.xy + float2(0, -offset), pixelDepth));
        lighting += float(U_texShadowMap.SampleCmpLevelZero(U_shadowMapSampler, shadowTexCoords.xy + float2(0, offset), pixelDepth));
        lighting += float(U_texShadowMap.SampleCmpLevelZero(U_shadowMapSampler, shadowTexCoords.xy + float2(-offset, 0), pixelDepth));
        lighting += float(U_texShadowMap.SampleCmpLevelZero(U_shadowMapSampler, shadowTexCoords.xy + float2(offset, 0), pixelDepth));
        lighting += float(U_texShadowMap.SampleCmpLevelZero(U_shadowMapSampler, shadowTexCoords.xy, pixelDepth));
 
        lighting *= 0.2f;
        return lighting;
    }

    return 1.0f;
}

//DirectionLight
float3 CalcDirectionLight(VS_OUT input, float3 f3CameraDir, float3 f3LightDir, float3 f3Normal, float3 f3DiffColor, float3 f3SpecColor)
{
    float fNdotL = dot(f3Normal, -f3LightDir);

    if (fNdotL > 0)
    {
        float shadow = ShadowCalculation(input.dirLightSpacePos, fNdotL);

        float3 fReflectDir = reflect(f3LightDir, f3Normal);
        float fSpec = pow(max(dot(f3CameraDir, fReflectDir), 0.0f), U_mat.fShininess);

        return float3(     U_dirLight.f3Diffuse * U_mat.f3Diffuse * fNdotL * f3DiffColor
                        + U_dirLight.f3Specular * U_mat.f3Specular * fSpec * f3SpecColor
                    ) * shadow;
    }
    else
    {
        return float3(0, 0, 0);
    }
}

float4 PS(VS_OUT input) : SV_Target
{
    
    float4 f3AmbiColor = U_mat.texDiffuse.Sample(U_defaultSampler, input.uv);

    float4 f3DiffColor = f3AmbiColor;
    float4 f3SpecColor = f3AmbiColor;
    
    if (U_mat.bUseSpecularTex > 0.5f)
    {
        f3SpecColor = U_mat.texSpecular.Sample(U_defaultSampler, input.uv);
    }

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
    float fAlpha = saturate(U_mat.fAlphaFactor) * f3AmbiColor.w;

    if (U_mat.bUseTransparentTex > 0.5f)
    {
        float4 transparentColor = U_mat.texTransparent.Sample(U_defaultSampler, input.uv);
        fAlpha *= transparentColor.a;

        clip(fAlpha - 0.1f);

#ifdef MTRANSPARENT_DEPTH_PEELING
        float2 f2DepthFrontUV = input.pos.xy;
        if (saturate(f2DepthFrontUV.x) == f2DepthFrontUV.x && saturate(f2DepthFrontUV.y) == f2DepthFrontUV.y)
        {   
            float fZDepth = input.pos.z;
            float bLessEqualFront = U_texDepthFront.SampleCmpLevelZero(U_shadowMapSampler, f2DepthFrontUV.xy, fZDepth);
            clip(bLessEqualFront);
        }
#endif
    }

    if(U_bDirectionLightEnabled > 0.5f)
    {
        f3Color += CalcDirectionLight(input, f3CameraDir, f3DirLightDir, f3Normal, f3DiffColor, f3SpecColor);
    }
    
    f3Color += input.vertexPointLight;

    for(int i = 0; i < min(MPOINT_LIGHT_PIXEL_NUMBER, U_nValidPointLightsNumber); ++i)
    {
//如果在VS处理Normal
#if MCALC_NORMAL_IN_VS
        float3 f3LightDir = input.pointLightDirTangentSpace[i];
#else
        float3 f3LightDir = normalize(U_pointLights[i].f3WorldPosition - input.worldPos);
#endif
        f3Color += CalcPointLight(U_pointLights[i], f3CameraDir, f3LightDir, f3Normal, input.worldPos, f3DiffColor, f3SpecColor);
    }

    for(int i = 0; i < min(MSPOT_LIGHT_PIXEL_NUMBER, U_nValidSpotLightsNumber); ++i)
    {
//如果在VS处理Normal
#if MCALC_NORMAL_IN_VS
        float3 f3LightDir = input.spotLightDirTangentSpace[i];
#else
        float3 f3LightDir = normalize(U_spotLights[i].f3WorldPosition - input.worldPos);
#endif
        f3Color += CalcSpotLight(U_spotLights[i], f3CameraDir, f3LightDir, f3Normal, input.worldPos, f3DiffColor, f3SpecColor);
    }

    return float4(f3Color, fAlpha);
}

