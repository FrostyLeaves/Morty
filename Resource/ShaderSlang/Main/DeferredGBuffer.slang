/*
    Deferred Rendering GBuffer Stage
*/


import InternalModule;
import Model.ModelModule;
import Deferred.DeferredModule;

[[vk::binding(0, 0)]] ParameterBlock<MaterialBatchData> materialDataBatch;

public struct PixelOut
{
    public float4 albedoMetalness : SV_Target0;
    public float4 normalRoughness : SV_Target1;
    public float4 ao : SV_Target2;
};


[shader("vertex")]
VertexOut VS_MAIN(VertexIn input, uint INSTANCE_ID: SV_InstanceID)
{
    StorageBatchMeshData instanceProxy;

    FrameData frameData = GlobalData.frame;

    VertexOut output = ModelUtil.ComputeVertex(input, instanceProxy, frameData.mainCamera, INSTANCE_ID);

    return output;
}

[shader("pixel")]
PixelOut PS_MAIN(VertexOut input)
{
    ModelMaterial material;
    material.materialDataBatch = materialDataBatch;

    var pixelData = ModelUtil.ComputePixelData(input);
    var surfaceData = material.GetSurfaceData(pixelData);

    GBufferPixelData gbufferData = DeferredUtil.ComputePixel(surfaceData);

    PixelOut output;
    output.albedoMetalness = gbufferData.albedoMetalness;
    output.normalRoughness = gbufferData.normalRoughness;
    output.ao = gbufferData.ao;

    return output;
}