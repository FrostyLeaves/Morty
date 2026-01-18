implementing DeferredModule;

public struct GBufferLoader
{
    public Texture2D<float4> gBuffer0;
    public Texture2D<float4> gBuffer1;
    public Texture2D<float4> gBuffer2;
    public Texture2D<float> depthTexture; // Depth texture

    // Matrix for position reconstruction
    public float4x4 invViewProjMatrix;

    public GBufferPixelData GetGBufferData(float2 screenUV)
    {
        GBufferPixelData gbufferData;
        gbufferData.albedoMetalness = gBuffer0.Sample(GlobalData.NearestSampler, screenUV);
        gbufferData.normalRoughness = gBuffer1.Sample(GlobalData.NearestSampler, screenUV);
        gbufferData.ao = gBuffer2.Sample(GlobalData.NearestSampler, screenUV).r;

        return gbufferData;
    }


    public SurfaceData LoadGBuffer(float2 screenUV)
    {
        FrameData frameData = GlobalData.frame;

        GBufferPixelData gbufferData = GetGBufferData(screenUV);
        
        // Sample depth
        float depth = depthTexture.Sample(GlobalData.NearestSampler, screenUV).r;

        SurfaceData surfaceData;
        surfaceData.albedo = gbufferData.albedoMetalness.xyz;
        surfaceData.metalness = gbufferData.albedoMetalness.w;

        surfaceData.normalWS = gbufferData.normalRoughness.xyz;
        surfaceData.roughness = gbufferData.normalRoughness.w;

        // Reconstruct world space position from depth
        surfaceData.positionWS = DeferredUtil.ReconstructPositionWS(depth, screenUV, invViewProjMatrix);
        surfaceData.ao = gbufferData.ao.r;

        surfaceData.cameraDir = normalize(frameData.mainCamera.positionWS - surfaceData.positionWS);

        return surfaceData;
    }
}
