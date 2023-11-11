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

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "Render/Vulkan/MVulkanWrapper.h"
#endif


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
	static constexpr int VOXEL_DIFFUSE_CONE_COUNT = 16;

	static MStringId SHADER_SKELETON_ENABLE;
	static MStringId DRAW_MESH_INSTANCING_NONE;
	static MStringId DRAW_MESH_INSTANCING_UNIFORM;
	static MStringId DRAW_MESH_INSTANCING_STORAGE;
	static MStringId MEN_TRANSPARENT;
	static MString SHADER_DEFINE_ENABLE_FLAG;
	static MString SHADER_DEFINE_DISABLE_FLAG;


	
	static constexpr size_t MESH_TRANSFORM_IN_UNIFORM_MAX_NUM = 128;

};

struct MORTY_API MShaderPropertyName
{
	//Frame
	static MStringId CBUFFER_FRAME_DATA;
	static MStringId CBUFFER_LIGHT_DATA;
	static MStringId CBUFFER_VOXEL_MAP_DATA;
	static MStringId TEXTURE_SHADOW_MAP;
	static MStringId TEXTURE_IRRADIANCE_MAP;
	static MStringId TEXTURE_PREFILTER_MAP;
	static MStringId TEXTURE_BRDF_LUT;
	static MStringId STORAGE_BONES_MATRIX;
	static MStringId STORAGE_BONES_OFFSET;
	static MStringId STORAGE_VOXEL_TABLE;

	//Frame Constant
	static MStringId FRAME_VIEW_MATRIX;
	static MStringId FRAME_CAMERA_PROJ_MATRIX;
	static MStringId FRAME_INV_CAMERA_PROJ_MATRIX;
	static MStringId FRAME_CAMERA_POSITION;
	static MStringId FRAME_CAMERA_DIRECTION;
	static MStringId FRAME_VIEWPORT_SIZE;
	static MStringId FRAME_Z_NEAR_FAR;
	static MStringId FRAME_TIME_DELTA;
	static MStringId FRAME_GAME_TIME;
	
	//Lighting
	static MStringId LIGHT_ENVIRONMENT_MAP_ENABLE;
	static MStringId LIGHT_DIRECTION_LIGHT_ENABLE;
	static MStringId LIGHT_DIRECTION_STRUCT_NAME;
	static MStringId LIGHT_DIRECTION_LIGHT_DIR;
	static MStringId LIGHT_INTENSITY;
	static MStringId LIGHT_DIRECTION_LIGHT_SIZE;

	static MStringId LIGHT_POINT_ARRAY_NAME;
	static MStringId LIGHT_POINT_POSITION;
	static MStringId LIGHT_POINT_CONSTANT;
	static MStringId LIGHT_POINT_LINEAR;
	static MStringId LIGHT_POINT_QUADRATIC;
	static MStringId LIGHT_POINT_COUNT;
	
	//Shadow
	static MStringId SHADOW_LIGHT_PROJ_MATRIX;
	static MStringId SHADOW_LIGHT_CASCADE_SPLIT;
	static MStringId SHADOW_GENERATE_CBUFFER_MATRIX_NAME;

	//Voxel
	static MStringId VOXEL_MAP_SETTING;
	static MStringId VOXEL_MAP_ORIGIN;
	static MStringId VOXEL_MAP_RESOLUTION;
	static MStringId VOXEL_MAP_VIEWPORT_SIZE;
	static MStringId VOXEL_MAP_STEP_SIZE;
	static MStringId VOXEL_MAP_CLIPMAP_ARRAY;
	static MStringId VOXEL_MAP_CLIPMAP_INDEX;
	static MStringId VOXELIZER_CAMERA_PROJ_MATRIX;
	static MStringId VOXELIZER_CBUFFER_VOXEL_MAP_NAME;
	static MStringId VOXELIZER_CUBE_MESH_INDEX;
	static MStringId VOXELIZER_CUBE_MESH_COUNT;
	static MStringId VOXELIZER_VOXEL_TEXTURE_NAME;
	static MStringId VOXELIZER_VOXEL_TABLE_NAME;

	//Transparent
	static MStringId TRANSPARENT_TEXTURE_INPUT_0;
	static MStringId TRANSPARENT_TEXTURE_INPUT_1;

	//Mesh Matrix
	static MStringId CBUFFER_MESH_MATRIX;
	static MStringId MESH_LOCAL_MATRIX;
	static MStringId MESH_WORLD_MATRIX;
	static MStringId MESH_NORMAL_MATRIX;
	static MStringId MESH_INSTANCE_INDEX;
	static MStringId MESH_INSTANCE_BEGIN_INDEX;

	//Material
	static MStringId MATERIAL_CBUFFER_NAME;
	static MStringId MATERIAL_STRUCT_NAME;
	static MStringId MATERIAL_METALLIC;
	static MStringId MATERIAL_ROUGHNESS;
	static MStringId MATERIAL_ALBEDO;
	static MStringId MATERIAL_AMBIENT;
	static MStringId MATERIAL_DIFFUSE;
	static MStringId MATERIAL_SPECULAR;
	static MStringId MATERIAL_SHININESS;
	static MStringId MATERIAL_NORMAL_TEXTURE_ENABLE;
	static MStringId MATERIAL_ALPHA_FACTOR;
	static MStringId MATERIAL_TEXTURE_DIFFUSE;
	static MStringId MATERIAL_TEXTURE_SPECULAR;
	static MStringId MATERIAL_TEXTURE_ALBEDO;
	static MStringId MATERIAL_TEXTURE_NORMAL;
	static MStringId MATERIAL_TEXTURE_METALLIC;
	static MStringId MATERIAL_TEXTURE_ROUGHNESS;
	static MStringId MATERIAL_TEXTURE_AMBIENTOCC;
	static MStringId MATERIAL_TEXTURE_HEIGHT;
	static MStringId MATERIAL_TEXTURE_EMISSION;


	//GBuffer
	static MStringId GBUFFER_TEXTURE_ALBEDO_METALLIC;
	static MStringId GBUFFER_TEXTURE_NORMAL_ROUGHNESS;
	static MStringId GBUFFER_TEXTURE_POSITION_AMBIENTOCC;
	static MStringId GBUFFER_TEXTURE_DEPTH_MAP;

	//GPU Culling
	static MStringId CULLING_INSTANCE_DATA;
	static MStringId CULLING_OUTPUT_DRAW_DATA;
	static MStringId CULLING_OUTPUT_DRAW_COUNT;
	static MStringId CULLING_CBUFFER_FRAME_DATA;
	static MStringId CULLING_CAMERA_POSITION;
	static MStringId CULLING_FRUSTUM_PLANES;

	//Environment
	static MStringId ENVIRONMENT_TEXTURE_SKYBOX;
	static MStringId ENVIRONMENT_IBL_MVP_MATRIX;
	static MStringId ENVIRONMENT_IBL_ROUGHNESS;

	//PostProcess
	static MStringId POSTPROCESS_SCREEN_TEXTURE;

	//ImGUI
	static MStringId IMGUI_SCALE;
	static MStringId IMGUI_TRANSLATE;
	static MStringId IMGUI_IMAGE_TYPE;
	static MStringId IMGUI_IMAGE_ARRAY;
	static MStringId IMGUI_IMAGE_INDEX;

};

using MMeshInstanceKey = int32_t;
using MSkeletonInstanceKey = int32_t;
