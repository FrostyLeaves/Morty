#if DRAW_MESH_INSTANCING_UNIFORM

    struct MeshMatrix
    {
        float4x4 u_matWorld;
        float3x3 u_matNormal;
    };

    //VS    per mesh
    [[vk::binding(0,2)]] cbuffer u_meshMatrix
    {
        MeshMatrix u_meshMatrix[MESH_TRANSFORM_IN_UNIFORM_MAX_NUM];
        int u_meshInstanceBeginIndex;
    }

    #define MESH_WORLD_MATRIX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_matWorld
    #define MESH_NORMAL_MATRIX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_matNormal

#elif DRAW_MESH_INSTANCING_STORAGE
    
    struct MeshMatrix
    {
        float4x4 u_matWorld;
        float3x3 u_matNormal;
    };

    [[vk::binding(0,2)]] StructuredBuffer<MeshMatrix> u_meshMatrix;
    [[vk::binding(1,2)]] cbuffer u_meshMatrix
    {
        int u_meshInstanceBeginIndex;
    }

    #define MESH_WORLD_MATRIX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_matWorld
    #define MESH_NORMAL_MATRIX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_matNormal

#else

    //VS    per mesh
    [[vk::binding(0,2)]]cbuffer u_meshMatrix : register(b0)
    {
        float4x4 u_matWorld;
        float3x3 u_matNormal;

        int u_meshInstanceBeginIndex;
    };

    #define MESH_WORLD_MATRIX u_matWorld
    #define MESH_NORMAL_MATRIX u_matNormal

#endif

//VS    with bones
[[vk::binding(0,3)]]cbuffer _M_E_cbAnimation : register(b2)
{
    float4x4 u_vBonesMatrix[128];
};


////////////////////////////////////////////////////////////////

struct LightBasicInfo
{
    float3 f3Normal;
    float3 f3CameraDir;
    float3 f3DirLightDir;
};
