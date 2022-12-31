#ifdef DRAW_MESH_MERGE_INSTANCING

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


#ifdef DRAW_MESH_MERGE_INSTANCING
    #define MESH_WORLD_MATRIX u_meshMatrix[u_meshClusterIndex[INSTANCE_ID - u_meshInstanceBeginIndex]].matWorld
#else
    #define MESH_WORLD_MATRIX u_matWorld
#endif



#ifdef DRAW_MESH_MERGE_INSTANCING
    #define MESH_NORMAL_MATRIX u_meshMatrix[u_meshClusterIndex[INSTANCE_ID - u_meshInstanceBeginIndex]].matNormal
#else
    #define MESH_NORMAL_MATRIX u_matNormal
#endif