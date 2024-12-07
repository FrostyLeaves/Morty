#ifndef _M_UNIVERSAL_MODEL_CBUFFER_HLSL_
#define _M_UNIVERSAL_MODEL_CBUFFER_HLSL_

struct MeshMatrix
{
    float4x4 u_matWorld;
    float3x3 u_matNormal;
    float4 u_meshIdx;
};

#define MESH_MATRIX_STRUCT MeshMatrix


#endif