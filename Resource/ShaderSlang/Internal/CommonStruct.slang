implementing InternalModule;

__include PreDefine;

public enum LightType: uint8_t
{
    Directional = 0,
    Spot,
    Point,
}

//all light use same struct.
// extension in point light: x --> constant, y --> linear, z --> quadratic
// extension in spot  light: x --> half inner cut off, y --> half outer cut off 
public struct LightData
{
    public float3 intensity;
    public LightType type;
    public float3 dir;
    public float3 positionWS;
    public float3 extension;
};

public struct SurfaceData
{
    public float3 albedo;
    public float roughness;
    public float3 normalWS;
    public float metalness;
    public float3 positionWS;
    public float ao;

    public float3 cameraDir;
};

public struct IndexedIndirectCommand
{
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    uint vertexOffset;
    uint firstInstance;
};

public struct CameraData
{
    public float4x4 viewMatrix;
    public float3 positionWS;
}

public struct FrameData
{
    public CameraData mainCamera;
}

public struct PixelData
{
    public float3 positionWS;
    public float2 uv;

    public float3x3 TBN;
}

public struct LightingOutput
{
    public float3 color;
}