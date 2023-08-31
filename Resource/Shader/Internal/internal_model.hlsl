struct VS_IN
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float2 uv : TEXCOORDS;
    
#if SKELETON_ENABLE == 1
    int bonesID[MBONES_PER_VERTEX] : BONES_ID;
    float bonesWeight[MBONES_PER_VERTEX] : BONES_WEIGHT;
#endif

};


#if DRAW_MESH_INSTANCING_UNIFORM

    struct MeshMatrix
    {
        float4x4 u_matWorld;
        float3x3 u_matNormal;
        float4 u_meshIdx;
    };

    //VS    per mesh
    [[vk::binding(0,2)]] cbuffer u_meshMatrix
    {
        MeshMatrix u_meshMatrix[MESH_TRANSFORM_IN_UNIFORM_MAX_NUM];
        int u_meshInstanceBeginIndex;
    }

    #define MESH_WORLD_MATRIX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_matWorld
    #define MESH_NORMAL_MATRIX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_matNormal
    #define MESH_INSTANCE_IDX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_meshIdx.x
    #define MESH_SKELETAL_IDX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_meshIdx.y

#elif DRAW_MESH_INSTANCING_STORAGE
    
    struct MeshMatrix
    {
        float4x4 u_matWorld;
        float3x3 u_matNormal;
        float4 u_meshIdx;
    };

    [[vk::binding(0,2)]] StructuredBuffer<MeshMatrix> u_meshMatrix;
    [[vk::binding(1,2)]] cbuffer u_meshMatrix
    {
        int u_meshInstanceBeginIndex;
    }

    #define MESH_WORLD_MATRIX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_matWorld
    #define MESH_NORMAL_MATRIX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_matNormal
    #define MESH_INSTANCE_IDX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_meshIdx.x
    #define MESH_SKELETAL_IDX u_meshMatrix[INSTANCE_ID - u_meshInstanceBeginIndex].u_meshIdx.y

#else

    //VS    per mesh
    [[vk::binding(0,2)]]cbuffer u_meshMatrix : register(b0)
    {
        float4x4 u_matWorld;
        float3x3 u_matNormal;
        float4 u_meshIdx;

        int u_meshInstanceBeginIndex;
    };

    #define MESH_WORLD_MATRIX u_matWorld
    #define MESH_NORMAL_MATRIX u_matNormal
    #define MESH_INSTANCE_IDX u_meshIdx.x
    #define MESH_SKELETAL_IDX u_meshIdx.y

#endif

//VS bones
[[vk::binding(0,3)]] StructuredBuffer<float4x4> u_vBonesMatrix;
[[vk::binding(1,3)]] StructuredBuffer<int> u_vBonesOffset;



float4 getModelVertexPosition(VS_IN input, uint INSTANCE_ID)
{
    
#if SKELETON_ENABLE == 1

    float4x4 matBoneTransform = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    float fWeight = 1.0f;
    for (int i = 0; i < MBONES_PER_VERTEX; ++i)
    {
        matBoneTransform += u_vBonesMatrix[input.bonesID[i] + u_vBonesOffset[MESH_SKELETAL_IDX]] * input.bonesWeight[i];
        fWeight = fWeight - input.bonesWeight[i];
    }
    matBoneTransform += float4x4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1) * fWeight;

    float4 vertexPos = mul(float4(input.pos, 1.0f), matBoneTransform);

#else

    float4 vertexPos = float4(input.pos, 1.0f);

#endif

    return vertexPos;
    
}
////////////////////////////////////////////////////////////////