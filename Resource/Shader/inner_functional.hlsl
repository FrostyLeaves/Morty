

float4 FloatToFloat4(float depth)
{
    const float4 bit_shift = float4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);
    const float4 bit_mask  = float4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);
    float4 res = frac(depth * bit_shift);
    res -= res.xxyz * bit_mask;
    return res;
}

float Float4ToFloat(float4 rgba_depth)
{
    const float4 bit_shift = float4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    float depth = dot(rgba_depth, bit_shift);
    return depth;
}


//https://stackoverflow.com/questions/35486775/how-do-i-convert-between-float-and-vec4-vec3-vec2

float2 EncodeExpV2( float value )
{
    int exponent  = int( log2( abs( value ) ) + 1.0 );
    value        /= exp2( float( exponent ) );
    value         = (value + 1.0) * 255.0 / (2.0*256.0);
    float2 encode   = frac( value * float2(1.0, 256.0) );
    return float2( encode.x - encode.y / 256.0 + 1.0/512.0, (float(exponent) + 127.5) / 256.0 );
}

float3 EncodeExpV3( float value )
{
    int exponent  = int( log2( abs( value ) ) + 1.0 );
    value        /= exp2( float( exponent ) );
    value         = (value + 1.0) * (256.0*256.0 - 1.0) / (2.0*256.0*256.0);
    float3 encode   = frac( value * float3(1.0, 256.0, 256.0*256.0) );
    return float3( encode.xy - encode.yz / 256.0 + 1.0/512.0, (float(exponent) + 127.5) / 256.0 );
}

float4 EncodeExpV4( float value )
{
    int exponent  = int( log2( abs( value ) ) + 1.0 );
    value        /= exp2( float( exponent ) );
    value         = (value + 1.0) * (256.0*256.0*256.0 - 1.0) / (2.0*256.0*256.0*256.0);
    float4 encode   = frac( value * float4(1.0, 256.0, 256.0*256.0, 256.0*256.0*256.0) );
    return float4( encode.xyz - encode.yzw / 256.0 + 1.0/512.0, (float(exponent) + 127.5) / 256.0 );
}

float DecodeExpV2( float2 pack )
{
    int exponent = int( pack.y * 256.0 - 127.0 );
    float value  = pack.x * (2.0*256.0) / 255.0 - 1.0;
    return value * exp2( float(exponent) );
}

float DecodeExpV3( float3 pack )
{
    int exponent = int( pack.z * 256.0 - 127.0 );
    float value  = dot( pack.xy, 1.0 / float2(1.0, 256.0) );
    value        = value * (2.0*256.0*256.0) / (256.0*256.0 - 1.0) - 1.0;
    return value * exp2( float(exponent) );
}

float DecodeExpV4( float4 pack )
{
    int exponent = int( pack.w * 256.0 - 127.0 );
    float value  = dot( pack.xyz, 1.0 / float3(1.0, 256.0, 256.0*256.0) );
    value        = value * (2.0*256.0*256.0*256.0) / (256.0*256.0*256.0 - 1.0) - 1.0;
    return value * exp2( float(exponent) );
}

//shadow
float CalcShadow(Texture2D texShadowMap, float4 dirLightSpacePos, float fNdotL)
{
    float2 shadowTexCoords;
    shadowTexCoords.x = 0.5f + (dirLightSpacePos.x / dirLightSpacePos.w * 0.5f);

    //DirectX Y 1 -> 0
    shadowTexCoords.y = 0.5f - (dirLightSpacePos.y / dirLightSpacePos.w * 0.5f);
    //Vulkan Y 0 -> 1
//    shadowTexCoords.y = 0.5f + (dirLightSpacePos.y / dirLightSpacePos.w * 0.5f);
    
	if (saturate(shadowTexCoords.x) == shadowTexCoords.x && saturate(shadowTexCoords.y) == shadowTexCoords.y)
    {     
        float margin = acos(saturate(fNdotL));
        float epsilon = clamp(margin / 1.570796, 0.001f, 0.003f);

        float lighting = 0.0f;
        
        float pixelDepth = min(dirLightSpacePos.z / dirLightSpacePos.w, 1.0f);
        float shadowDepth = texShadowMap.Sample(U_defaultSampler, shadowTexCoords.xy);

        //current pixel is nearer.
        if (pixelDepth < shadowDepth + epsilon)
            lighting += 1.0f;

            
        return lighting;
    }

    return 1.0f;
}
