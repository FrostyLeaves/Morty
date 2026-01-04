implementing ModelModule;
#include "Internal/PreDefine.slang"


public struct ModelUtil
{
    public static float4 GetVertexPosition(VertexIn input, uint INSTANCE_ID)
    {

#if SKELETON_ENABLE

        float4x4 matBoneTransform = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        float fWeight = 1.0f;
        for (int i = 0; i < MBONES_PER_VERTEX; ++i)
        {
            matBoneTransform += u_vBonesMatrix[input.bonesID[i] + u_vBonesOffset[MESH_SKELETAL_IDX]] * input.bonesWeight[i];
            fWeight = fWeight - input.bonesWeight[i];
        }
        matBoneTransform += float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1) * fWeight;

        float4 vertexPos = mul(float4(input.pos, 1.0f), matBoneTransform);

#else

        float4 vertexPos = float4(input.pos, 1.0f);

#endif

        return vertexPos;
    }

    public static VertexOut ComputeVertex(VertexIn input, CameraData mainCameraData, uint INSTANCE_ID)
    {
        VertexOut output;

        StorageBatchMeshData model;

        MeshInstanceData data = model.GetMeshData(INSTANCE_ID);

        float4 positionCS = ModelUtil.GetVertexPosition(input, INSTANCE_ID);
        float4 positionWS = mul(positionCS, data.matWorld);

        output.pos = mul(positionWS, mainCameraData.viewMatrix);
        output.uv = input.uv;
        output.positionWS = positionWS.xyz;
        output.depth = output.pos.z;

        // matNormal computed dynamically: transpose(inverse(matWorld))
        float3x3 matNormal = transpose(inverse((float3x3)data.matWorld));
        output.normal = normalize(mul(input.normal, matNormal));
        output.tangent = normalize(mul(input.tangent, matNormal));
        output.bitangent = normalize(cross(output.tangent, output.normal));

        output.instanceId = INSTANCE_ID;

        return output;
    }

    public static PixelData ComputePixelData(VertexOut input)
    {
        float3 T = normalize(input.tangent);
        float3 B = normalize(input.bitangent);
        float3 N = normalize(input.normal);

        PixelData pixel;
        pixel.positionWS = input.positionWS;
        pixel.uv = input.uv;
        pixel.TBN = float3x3(T, B, N);
        pixel.instanceId = input.instanceId;

        return pixel;
    }
}