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
    static MStringId TEXTURE_SHADOW_MAP;

    //Voxel
    static MStringId VOXEL_MAP_SETTING;
    static MStringId VOXEL_MAP_ORIGIN;
    static MStringId VOXEL_MAP_RESOLUTION;
    static MStringId VOXEL_MAP_VIEWPORT_SIZE;
    static MStringId VOXEL_MAP_STEP_SIZE;
    static MStringId VOXEL_MAP_CLIPMAP_ARRAY;
    static MStringId VOXEL_MAP_CLIPMAP_INDEX;

    //Material
    static MStringId MATERIAL_METALLIC;
    static MStringId MATERIAL_ROUGHNESS;
    static MStringId MATERIAL_ALBEDO;
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
    static MStringId GBUFFER_TEXTURE_SSAO;

    //PostProcess
    static MStringId POSTPROCESS_SCREEN_TEXTURE[8];
    static MStringId POSTPROCESS_BLUR_OFFSET;

    //ImGUI
    static MStringId IMGUI_SCALE;
    static MStringId IMGUI_TRANSLATE;
    static MStringId IMGUI_IMAGE_TYPE;
    static MStringId IMGUI_SINGLE_CHANNEL_FLAG;
    static MStringId IMGUI_IMAGE_INDEX;
    static MStringId IMGUI_IMAGE_SIZE;
};

using MMaterialInstanceKey = size_t;
using MMeshInstanceKey     = size_t;
using MSkeletonInstanceKey = size_t;

}// namespace morty