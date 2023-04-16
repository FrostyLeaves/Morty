#if DRAW_MESH_INSTANCING_UNIFORM

    struct MeshMatrix
    {
        float4x4 matWorld;
        float3x3 matNormal;
    };

    //VS    per mesh
    [[vk::binding(0,2)]]cbuffer _M_E_cbMeshMatrix : register(b0)
    {
        MeshMatrix u_meshMatrix[MERGE_INSTANCING_MAX_NUM];
        int u_meshClusterIndex[MERGE_INSTANCING_CLUSTER_MAX_NUM];
        int u_meshInstanceBeginIndex;
    };

#elif DRAW_MESH_INSTANCING_STORAGE
    
    struct MeshMatrix
    {
        float4x4 matWorld;
        float3x3 matNormal;
    };

    [[vk::binding(0,2)]] StructuredBuffer<MeshMatrix> u_meshMatrix;

#else

    //VS    per mesh
    [[vk::binding(0,2)]]cbuffer _M_E_cbMeshMatrix : register(b0)
    {
        float4x4 u_matWorld;
        float3x3 u_matNormal;
    };

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

#if DRAW_MESH_INSTANCING_UNIFORM
    #define MESH_WORLD_MATRIX u_meshMatrix[u_meshClusterIndex[INSTANCE_ID - u_meshInstanceBeginIndex]].matWorld
#elif DRAW_MESH_INSTANCING_STORAGE
    #define MESH_WORLD_MATRIX u_meshMatrix[INSTANCE_ID].matWorld
#else
    #define MESH_WORLD_MATRIX u_matWorld
#endif

#if DRAW_MESH_INSTANCING_UNIFORM
    #define MESH_NORMAL_MATRIX u_meshMatrix[u_meshClusterIndex[INSTANCE_ID - u_meshInstanceBeginIndex]].matNormal
#elif DRAW_MESH_INSTANCING_STORAGE
    #define MESH_NORMAL_MATRIX u_meshMatrix[INSTANCE_ID].matNormal
#else
    #define MESH_NORMAL_MATRIX u_matNormal
#endif