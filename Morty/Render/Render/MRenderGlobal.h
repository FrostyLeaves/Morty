/**
 * @File         MIDevice
 * 
 * @Created      2021-7-7 14:20:55
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Utility/MStringId.h"
#include "MShaderPropertyName.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanWrapper.h"
#endif

#define GPU_CULLING_ENABLE (false)
#define MORTY_VXGI_ENABLE (false)
#define VRS_OPTIMIZE_ENABLE  (false)

class MORTY_API MRenderGlobal
{
public:


	static constexpr int SHADER_PARAM_SET_MATERIAL = 0;
	static constexpr int SHADER_PARAM_SET_FRAME = 1;
	static constexpr int SHADER_PARAM_SET_MESH = 2;
	static constexpr int SHADER_PARAM_SET_OTHER = 3;
	static constexpr int SHADER_PARAM_SET_NUM = 4;

	static constexpr int BONES_PER_VERTEX = 4;
	static constexpr int BONES_MAX_NUMBER = 128;
	static constexpr int SHADOW_TEXTURE_SIZE = 2048;
    static constexpr uint32_t VOXEL_TABLE_SIZE = 64;
	static constexpr uint32_t VOXEL_VIEWPORT_SIZE = VOXEL_TABLE_SIZE * 1;

	static constexpr int POINT_LIGHT_MAX_NUMBER = 8;
	static constexpr int POINT_LIGHT_PIXEL_NUMBER = 8;
	static constexpr int SPOT_LIGHT_MAX_NUMBER = 8;
	static constexpr int SPOT_LIGHT_PIXEL_NUMBER = 8;

	static constexpr int MESH_LOD_LEVEL_RANGE = 3;

	static const char* SUFFIX_VERTEX_SHADER;
	static const char* SUFFIX_PIXEL_SHADER;
	static const char* SUFFIX_COMPUTE_SHADER;
	static const char* SUFFIX_GEOMETRY_SHADER;

	static constexpr int CASCADED_SHADOW_MAP_NUM = 4;
	static constexpr int VOXEL_GI_CLIP_MAP_NUM = 6;
	static constexpr float VOXEL_BASIC_VOXEL_SIZE = 1.0f;
	static constexpr int VOXEL_DIFFUSE_CONE_COUNT = 16;

	static MStringId SHADER_SKELETON_ENABLE;
	static MStringId DRAW_MESH_INSTANCING_NONE;
	static MStringId DRAW_MESH_INSTANCING_UNIFORM;
	static MStringId DRAW_MESH_INSTANCING_STORAGE;
	static MStringId VOXELIZER_CONSERVATIVE_RASTERIZATION;
	static MStringId MEN_TRANSPARENT;
	static MString SHADER_DEFINE_ENABLE_FLAG;
	static MString SHADER_DEFINE_DISABLE_FLAG;


	
	static constexpr size_t MESH_TRANSFORM_IN_UNIFORM_MAX_NUM = 128;

	static MStringId TASK_RENDER_MESH_MANAGER_UPDATE;
	static MStringId TASK_ANIMATION_MANAGER_UPDATE;
	static MStringId TASK_SHADOWMAP_MANAGER_UPDATE;
	static MStringId TASK_UPLOAD_MESH_UPDATE;
	static MStringId TASK_RENDER_MODULE_UPDATE;

	static MStringId POSTPROCESS_FINAL_NODE;
	static MStringId POSTPROCESS_EDGE_DETECTION;
};

using MMeshInstanceKey = size_t;
using MSkeletonInstanceKey = size_t;
