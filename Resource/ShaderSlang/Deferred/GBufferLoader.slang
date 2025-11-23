implementing DeferredModule;

public struct GBufferLoader
{
    public Texture2D<float4> gBuffer0;
    public Texture2D<float4> gBuffer1;
    public Texture2D<float4> gBuffer2;
    public Texture2D<float> depthTexture; // 深度纹理
    
    // 用于位置重建的矩阵
    public float4x4 invViewProjMatrix;

    public GBufferPixelData GetGBufferData(float2 screenUV)
    {
        GBufferPixelData gbufferData;
        gbufferData.albedoMetalness = gBuffer0.Sample(NearestSampler, screenUV);
        gbufferData.normalRoughness = gBuffer1.Sample(NearestSampler, screenUV);
        gbufferData.ao = gBuffer2.Sample(NearestSampler, screenUV).r;

        return gbufferData;
    }


    public SurfaceData LoadGBuffer(float2 screenUV)
    {
        FrameData frameData = GlobalFrameData;

        GBufferPixelData gbufferData = GetGBufferData(screenUV);
        
        // 采样深度
        float depth = depthTexture.Sample(NearestSampler, screenUV).r;

        SurfaceData surfaceData;
        surfaceData.albedo = gbufferData.albedoMetalness.xyz;
        surfaceData.metalness = gbufferData.albedoMetalness.w;

        surfaceData.normalWS = gbufferData.normalRoughness.xyz;
        surfaceData.roughness = gbufferData.normalRoughness.w;

        // 从深度重建世界空间位置
        surfaceData.positionWS = DeferredUtil.ReconstructPositionWS(depth, screenUV, invViewProjMatrix);
        surfaceData.ao = gbufferData.ao.r;

        surfaceData.cameraDir = normalize(frameData.mainCamera.positionWS - surfaceData.positionWS);

        return surfaceData;
    }
}
