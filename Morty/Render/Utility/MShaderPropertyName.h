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

namespace morty
{

struct MORTY_API MShaderPropertyName {
    //Frame
    static MStringId CBUFFER_FRAME_DATA;
    static MStringId CBUFFER_LIGHT_DATA;
    static MStringId CBUFFER_VOXEL_MAP_DATA;
    static MStringId TEXTURE_SHADOW_MAP;
    static MStringId TEXTURE_IRRADIANCE_MAP;
    static MStringId TEXTURE_PREFILTER_MAP;
    static MStringId TEXTURE_BRDF_LUT;
    static MStringId TEXTURE_NOISE_TEX;
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
    static MStringId FRAME_VIEWPORT_SIZE_INV;
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
    static MStringId TRANSPARENT_TEXTURE_BACK_TEXTURE;

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
    static MStringId MATERIAL_METALLIC_CHANNEL;
    static MStringId MATERIAL_ROUGHNESS_CHANNEL;
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
    static MStringId GBUFFER_TEXTURE_SSAO;

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
    static MStringId POSTPROCESS_SCREEN_TEXTURE[8];
    static MStringId POSTPROCESS_SCREEN_SIZE;

    static MStringId POSTPROCESS_BLUR_OFFSET;

    //ImGUI
    static MStringId IMGUI_SCALE;
    static MStringId IMGUI_TRANSLATE;
    static MStringId IMGUI_IMAGE_TYPE;
    static MStringId IMGUI_SINGLE_CHANNEL_FLAG;
    static MStringId IMGUI_IMAGE_INDEX;
    static MStringId IMGUI_IMAGE_SIZE;

    //VRS
    static MStringId VRS_CBUFFER_SETTING_NAME;
    static MStringId VRS_EDGE_TEXTURE_NAME;
    static MStringId VRS_OUTPUT_VRS_TEXTURE_NAME;
    static MStringId VRS_EDGE_TEXTURE_SIZE_NAME;
    static MStringId VRS_TEXEL_SIZE_NAME;
    static MStringId VRS_EDGE_THRESHOLD_NAME;

    //HBAO
    static MStringId HBAO_NEAREST_AO_SCALE;
    static MStringId HBAO_OTHER_AO_SCALE;
    static MStringId HBAO_NDOTV_BIAS;
    static MStringId HBAO_RADIUS_PIXEL;
    static MStringId HBAO_RADIUS_UV_SQUARE_NEG_INV;
    static MStringId HBAO_UV_TO_VIEW;
};

using MMeshInstanceKey     = size_t;
using MSkeletonInstanceKey = size_t;

}// namespace morty