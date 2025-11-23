/*
    延迟渲染GBuffer阶段
*/


import InternalModule;
import Model.ModelModule;
import Deferred.DeferredModule;

public struct PixelOut
{
    public float4 albedoMetalness : SV_Target0;
    public float4 normalRoughness : SV_Target1;
    public float4 ao : SV_Target2;
};


[shader("vertex")]
VertexOut VS_MAIN(VertexIn input, uint INSTANCE_ID: SV_InstanceID)
{
    FrameData frameData = GlobalFrameData;

    VertexOut output = ModelUtil.ComputeVertex(input, frameData.mainCamera, INSTANCE_ID);

    return output;
}

[shader("pixel")]
PixelOut PS_MAIN(VertexOut input)
{
    ModelMaterial material;

    var pixelData = ModelUtil.ComputePixelData(input);
    var surfaceData = material.GetSurfaceData(pixelData);

    GBufferPixelData gbufferData = DeferredUtil.ComputePixel(surfaceData);

    PixelOut output;
    output.albedoMetalness = gbufferData.albedoMetalness;
    output.normalRoughness = gbufferData.normalRoughness;
    output.ao = gbufferData.ao;

    return output;
}