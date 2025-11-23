/**
 * @File         MVertex
 * 
 * @Created      2019-08-25 15:08:54
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Math/Vector.h"
#include "Utility/MString.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "RHI/Vulkan/MVulkanWrapper.h"
#endif

namespace morty
{

//顶点
struct MVertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoords;
    Vector3 tangent;
    Vector3 bitangent;
};

template<> inline size_t             AttributeProtectMask<MVertex>() { return 3 << 6; }
template<> inline std::vector<float> SimplifyWeight<MVertex>() { return {0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f}; }

//带骨骼顶点
struct MVertexWithBones {
    MVertexWithBones()
    {
        memset(bonesID, 0, sizeof(bonesID));
        memset(bonesWeight, 0, sizeof(bonesID));
    }
    Vector3 position;
    Vector3 normal;
    Vector2 texCoords;
    Vector3 tangent;
    Vector3 bitangent;

    int     bonesID[MRenderGlobal::BONES_PER_VERTEX];
    float   bonesWeight[MRenderGlobal::BONES_PER_VERTEX];
};

template<> inline size_t             AttributeProtectMask<MVertexWithBones>() { return 3 << 6; }
template<> inline std::vector<float> SimplifyWeight<MVertexWithBones>() { return {0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f}; }


struct MMeshInstanceTransform {
    Matrix4 transform;
    Matrix3 normalTransform;
    Vector4 instanceIndex;
};

struct MMergeInstanceCullData {
    Vector3 position;
    float   radius;

    struct LOD {
        size_t firstIndex = 0;
        size_t indexCount = 0;
        float  distance   = 0.0f;
        float  _pad0      = 0.0f;
    } lods[MRenderGlobal::MESH_LOD_LEVEL_RANGE];
};


#if RENDER_GRAPHICS == MORTY_VULKAN
typedef VkDrawIndexedIndirectCommand MDrawIndexedIndirectData;
#endif


struct MMergeInstanceDrawCallOutput {
    uint32_t drawCount                                     = 0;
    uint32_t lodCount[MRenderGlobal::MESH_LOD_LEVEL_RANGE] = {};
};

struct MShadowGpuDrivenOutput {
    uint32_t drawCount;
    uint32_t lodCount[MRenderGlobal::MESH_LOD_LEVEL_RANGE];
};


struct VoxelizerOutput {
    uint32_t nBaseColor[4 * 6];
    uint32_t nVoxelCount[6];
};

}// namespace morty