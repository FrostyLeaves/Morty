﻿/**
 * @File         MIDevice
 * 
 * @Created      2021-7-7 14:20:55
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanWrapper.h"
#endif


class MORTY_API MRenderGlobal
{
public:

	static const char* SHADER_SKELETON_ENABLE;

	static const char* MATERIAL_MACRO_SKELETON_ENABLE;

	static constexpr int SHADER_PARAM_SET_MATERIAL = 0;
	static constexpr int SHADER_PARAM_SET_FRAME = 1;
	static constexpr int SHADER_PARAM_SET_MESH = 2;
	static constexpr int SHADER_PARAM_SET_SKELETON = 3;
	static constexpr int SHADER_PARAM_SET_NUM = 4;

	static constexpr int BONES_PER_VERTEX = 4;
	static constexpr int BONES_MAX_NUMBER = 128;
	static const int SHADOW_TEXTURE_SIZE;
    static const int VOXEL_TABLE_SIZE;

	static constexpr int POINT_LIGHT_MAX_NUMBER = 8;
	static constexpr int POINT_LIGHT_PIXEL_NUMBER = 8;
	static constexpr int SPOT_LIGHT_MAX_NUMBER = 8;
	static constexpr int SPOT_LIGHT_PIXEL_NUMBER = 8;

	static constexpr int MESH_LOD_LEVEL_RANGE = 3;

	static const bool VERTEX_NORMAL = false;


	static const char* SUFFIX_VERTEX_SHADER;
	static const char* SUFFIX_PIXEL_SHADER;

	static const bool GBUFFER_UNIFIED_FORMAT;

	static constexpr int CASCADED_SHADOW_MAP_NUM = 4;

	static const char* DRAW_MESH_INSTANCING_UNIFORM;
	static const char* DRAW_MESH_INSTANCING_STORAGE;

	static const size_t MERGE_INSTANCING_MAX_NUM;
	static const size_t MERGE_INSTANCING_CLUSTER_MAX_NUM;
	static const size_t MESH_TRANSFORM_IN_UNIFORM_MAX_NUM;

};

typedef int32_t MMeshInstanceKey;
typedef int32_t MSkeletonInstanceKey;
