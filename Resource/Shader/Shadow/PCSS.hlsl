#include "inner_constant.hlsl"

#define NUM_SAMPLES 50
#define NUM_RINGS 10

#define NEAR_PLANE 0.01f

float Rand_UV(const float2 uv)
{ 
    const float a = 12.9898, b = 78.233, c = 43758.5453;
	float dt = dot( uv.xy, float2( a,b ) );
    float sn = fmod( dt, NUM_PI );
	return frac(sin(sn) * c);
}

float Rand(const float x )
{ 
  // -1 -1
  return frac(sin(x) * 10000.0);
}

void PoissonDiskSamples( const float2 f2RandomSeed, out float2 vPoissonDisk[NUM_SAMPLES] )
{
    float ANGLE_STEP = NUM_PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
    float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

    float angle = Rand_UV( f2RandomSeed ) * NUM_PI2;
    float radius = INV_NUM_SAMPLES;
    float radiusStep = radius;

    for ( int i = 0; i < NUM_SAMPLES; ++i )
    {
        vPoissonDisk[i] = float2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
        radius += radiusStep;
        angle += ANGLE_STEP;
    }
}

void UniformDiskSamples( const float2 f2RandomSeed, out float2 vPoissonDisk[NUM_SAMPLES] )
{
    float randNum = Rand_UV(f2RandomSeed);
    float sampleX = Rand_UV( randNum ) ;
    float sampleY = Rand_UV( sampleX ) ;

    float angle = sampleX * NUM_PI2;
    float radius = sqrt(sampleY);

    for( int i = 0; i < NUM_SAMPLES; ++i )
    {
        vPoissonDisk[i] = float2( radius * cos(angle) , radius * sin(angle)  );

        sampleX = Rand( sampleY ) ;
        sampleY = Rand( sampleX ) ;

        angle = sampleX * NUM_PI2;
        radius = sqrt(sampleY);
    }
}

float FindBlocker(Texture2DArray texShadowMap, uint nCascadeIndex, float2 f2TexCoords, float fLightSize, float4 f4DirLightSpacePos, float fReceiverDepth)
{
    float fBlockerPixelNum = 0.0f;
    float fBlockDepth = 0.0f;

    float fLightSpaceZ = f4DirLightSpacePos.z;

    //float fSearchRadius = 5.0f / MSHADOW_TEXTURE_SIZE;
    float fSearchRadius = (fLightSize / MSHADOW_TEXTURE_SIZE) * (fLightSpaceZ - NEAR_PLANE) / fLightSpaceZ;

    float2 vPoissonDisk[NUM_SAMPLES];
    PoissonDiskSamples(f2TexCoords, vPoissonDisk);

    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        float2 texCoords = saturate(f2TexCoords + vPoissonDisk[i] * fSearchRadius);

        float fShadowDepth = texShadowMap.Sample( LinearSampler, float3(texCoords, nCascadeIndex) ).r;

        //fShadowDepth < fReceiverDepth
        fBlockerPixelNum += (1.0f - step(fReceiverDepth, fShadowDepth));
        fBlockDepth += (1.0f - step(fReceiverDepth, fShadowDepth)) * fShadowDepth;
    }

    if ( fBlockerPixelNum < 1.0f )
    {
        return -1.0f;
    }

    return fBlockDepth / fBlockerPixelNum;
}

float PCF(Texture2DArray texShadowMap, uint nCascadeIndex, float2 f2TexCoords, float fPixelDepth, float fFilterRadius, float fEpsilon)
{
    float fVisibility = 0.0f;

    float fFilterRadiusUV = fFilterRadius / MSHADOW_TEXTURE_SIZE;

    float2 vPoissonDisk[NUM_SAMPLES];
    PoissonDiskSamples(f2TexCoords, vPoissonDisk);
        
    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        float2 texCoords = saturate(f2TexCoords + vPoissonDisk[i] * fFilterRadiusUV);

        float fShadowDepth = texShadowMap.Sample( LinearSampler, float3(texCoords, nCascadeIndex) ).r;

        fVisibility += step(fPixelDepth, fShadowDepth + fEpsilon);
    }

    return fVisibility / NUM_SAMPLES;
}

float PCSS(Texture2DArray texShadowMap, uint nCascadeIndex, float2 f2TexCoords, float fLightSize, float4 f4DirLightSpacePos, float fPixelDepth, float fEpsilon)
{
    float fReceiverDepth = fPixelDepth;

    // STEP 1: avgblocker depth 
    float fBlockerDepth = FindBlocker(texShadowMap, nCascadeIndex, f2TexCoords, fLightSize, f4DirLightSpacePos, fReceiverDepth);

    if(fBlockerDepth < 0.0f)
    {
        return 1.0f;
    }

    // STEP 2: penumbra size
    float fPenumbra = fLightSize * (fPixelDepth - fBlockerDepth) / fBlockerDepth;

    //fPenumbra = fPenumbra / (nCascadeIndex + 1);

    // STEP 3: filtering
    return PCF(texShadowMap, nCascadeIndex, f2TexCoords, fPixelDepth, fPenumbra, fEpsilon);
}
