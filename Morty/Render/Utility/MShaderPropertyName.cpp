#include "MShaderPropertyName.h"

using namespace morty;

MStringId MShaderPropertyName::MATERIAL_METALLIC = MStringId("metalness");
MStringId MShaderPropertyName::MATERIAL_ROUGHNESS = MStringId("roughness");
MStringId MShaderPropertyName::MATERIAL_ALBEDO = MStringId("albedo");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_ALBEDO = MStringId("albedoTexture");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_NORMAL = MStringId("normalTexture");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_METALLIC = MStringId("metallicTexture");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_ROUGHNESS = MStringId("roughnessTexture");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_AMBIENTOCC = MStringId("aoTexture");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_HEIGHT = MStringId("heightTexture");

MStringId MShaderPropertyName::TEXTURE_SHADOW_MAP = MStringId("u_texShadowMap");
MStringId MShaderPropertyName::VOXEL_MAP_SETTING = MStringId("voxelMapSetting");
MStringId MShaderPropertyName::VOXEL_MAP_ORIGIN = MStringId("f3VoxelOrigin");
MStringId MShaderPropertyName::VOXEL_MAP_RESOLUTION = MStringId("nResolution");
MStringId MShaderPropertyName::VOXEL_MAP_VIEWPORT_SIZE = MStringId("nViewportSize");
MStringId MShaderPropertyName::VOXEL_MAP_STEP_SIZE = MStringId("fVoxelSize");
MStringId MShaderPropertyName::VOXEL_MAP_CLIPMAP_ARRAY = MStringId("vClipmap");
MStringId MShaderPropertyName::VOXEL_MAP_CLIPMAP_INDEX = MStringId("nClipmapIdx");
MStringId MShaderPropertyName::MATERIAL_DIFFUSE = MStringId("f3Diffuse");
MStringId MShaderPropertyName::MATERIAL_SPECULAR = MStringId("f3Specular");
MStringId MShaderPropertyName::MATERIAL_SHININESS = MStringId("fShininess");
MStringId MShaderPropertyName::MATERIAL_NORMAL_TEXTURE_ENABLE = MStringId("bUseNormalTex");
MStringId MShaderPropertyName::MATERIAL_ALPHA_FACTOR = MStringId("fAlphaFactor");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_DIFFUSE = MStringId("u_texDiffuse");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_SPECULAR = MStringId("u_texSpecular");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_EMISSION = MStringId("u_mat_texEmission");
MStringId MShaderPropertyName::GBUFFER_TEXTURE_ALBEDO_METALLIC = MStringId("u_mat_f3Albedo_fMetallic");
MStringId MShaderPropertyName::GBUFFER_TEXTURE_NORMAL_ROUGHNESS = MStringId("u_mat_f3Normal_fRoughness");
MStringId MShaderPropertyName::GBUFFER_TEXTURE_POSITION_AMBIENTOCC = MStringId("u_mat_f3Position_fAmbientOcc");
MStringId MShaderPropertyName::GBUFFER_TEXTURE_SSAO = MStringId("u_mat_SSAO");
MStringId MShaderPropertyName::POSTPROCESS_SCREEN_TEXTURE[8] = {
        MStringId("u_texInputTexture"),
        MStringId("u_texInputTexture1"),
        MStringId("u_texInputTexture2"),
        MStringId("u_texInputTexture3"),
        MStringId("u_texInputTexture4"),
        MStringId("u_texInputTexture5"),
        MStringId("u_texInputTexture6"),
        MStringId("u_texInputTexture7"),
};
MStringId MShaderPropertyName::POSTPROCESS_BLUR_OFFSET = MStringId("u_f2GaussianBlurOffset");
MStringId MShaderPropertyName::IMGUI_SCALE = MStringId("u_f2Scale");
MStringId MShaderPropertyName::IMGUI_TRANSLATE = MStringId("u_f2Translate");
MStringId MShaderPropertyName::IMGUI_IMAGE_TYPE = MStringId("u_nImageType");
MStringId MShaderPropertyName::IMGUI_SINGLE_CHANNEL_FLAG = MStringId("u_nSingleChannelFlag");
MStringId MShaderPropertyName::IMGUI_IMAGE_INDEX = MStringId("u_nImageIndex");
MStringId MShaderPropertyName::IMGUI_IMAGE_SIZE = MStringId("u_nImageSize");
