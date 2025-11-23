import InternalModule;
import Model.ModelModule;
import Forward.ForwardModule;
import Lighting.LightingModule;

public struct PixelOut
{
    public float4 color : SV_Target0;
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
    FrameData frameData = GlobalFrameData;
    
    ModelMaterial material;
    var pixelData = ModelUtil.ComputePixelData(input);
    var surfaceData = material.GetSurfaceData(pixelData);
    
    // Compute camera direction
    surfaceData.cameraDir = normalize(frameData.mainCamera.positionWS - surfaceData.positionWS);
    
    // Forward lighting computation
    float3 lighting = ForwardLighting.ComputeForwardLighting(surfaceData, 16);
    
    // Ambient occlusion
    lighting *= surfaceData.ao;
    
    PixelOut output;
    output.color = float4(lighting, 1.0);
    return output;
}
