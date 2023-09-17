/**
 * @File         MVertex
 * 
 * @Created      2019-08-25 15:08:54
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Render/MRenderGlobal.h"
#include "Math/Vector.h"
#include "Utility/MString.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanWrapper.h"
#endif

//顶点
struct MVertex
{
	Vector3 position;
	Vector3 normal;
	Vector3 tangent;
	Vector3 bitangent;
	Vector2 texCoords;
};

//带骨骼顶点
struct MVertexWithBones
{
	MVertexWithBones()
	{
		memset(bonesID, 0, sizeof(bonesID));
		memset(bonesWeight, 0, sizeof(bonesID));
	}
	Vector3 position;
	Vector3 normal;
	Vector3 tangent;
	Vector3 bitangent;
	Vector2 texCoords;


	int bonesID[MRenderGlobal::BONES_PER_VERTEX];
	float bonesWeight[MRenderGlobal::BONES_PER_VERTEX];
};

struct MMeshInstanceTransform
{
	Matrix4 transform;
	Matrix3 normalTransform;
	Vector4 instanceIndex;
};

struct MMergeInstanceCullData
{
	Vector3 position;
	float radius;

	struct LOD
	{
		uint32_t firstIndex;
		uint32_t indexCount;
		float distance;
		float _pad0;
	} lods[MRenderGlobal::MESH_LOD_LEVEL_RANGE];
};


#if RENDER_GRAPHICS == MORTY_VULKAN
typedef VkDrawIndexedIndirectCommand MDrawIndexedIndirectData;
#endif


struct MMergeInstanceDrawCallOutput
{
	uint32_t drawCount = 0;
	uint32_t lodCount[MRenderGlobal::MESH_LOD_LEVEL_RANGE] = {};
};

struct MShadowGpuDrivenOutput
{
	uint32_t drawCount;
	uint32_t lodCount[MRenderGlobal::MESH_LOD_LEVEL_RANGE];
};


struct VoxelizerOutput
{
	uint32_t nVoxelCount;
};