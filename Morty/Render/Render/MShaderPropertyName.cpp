#include "Render/MShaderPropertyName.h"

MStringId MShaderPropertyName::CBUFFER_FRAME_DATA = MStringId("cbFrameInformation");
MStringId MShaderPropertyName::CBUFFER_LIGHT_DATA = MStringId("cbLightInformation");
MStringId MShaderPropertyName::CBUFFER_VOXEL_MAP_DATA = MStringId("cbVoxelMapInformation");
MStringId MShaderPropertyName::TEXTURE_SHADOW_MAP = MStringId("u_texShadowMap");
MStringId MShaderPropertyName::TEXTURE_IRRADIANCE_MAP = MStringId("u_texIrradianceMap");
MStringId MShaderPropertyName::TEXTURE_PREFILTER_MAP = MStringId("u_texPrefilterMap");
MStringId MShaderPropertyName::TEXTURE_BRDF_LUT = MStringId("u_texBrdfLUT");
MStringId MShaderPropertyName::STORAGE_BONES_MATRIX = MStringId("u_vBonesMatrix");
MStringId MShaderPropertyName::STORAGE_BONES_OFFSET = MStringId("u_vBonesOffset");
MStringId MShaderPropertyName::STORAGE_VOXEL_TABLE = MStringId("u_rwVoxelTable");

MStringId MShaderPropertyName::FRAME_VIEW_MATRIX = MStringId("u_matView");
MStringId MShaderPropertyName::FRAME_CAMERA_PROJ_MATRIX = MStringId("u_matCamProj");
MStringId MShaderPropertyName::FRAME_INV_CAMERA_PROJ_MATRIX = MStringId("u_matCamProjInv");
MStringId MShaderPropertyName::FRAME_CAMERA_POSITION = MStringId("u_f3CameraPosition");
MStringId MShaderPropertyName::FRAME_CAMERA_DIRECTION = MStringId("u_f3CameraDirection");
MStringId MShaderPropertyName::FRAME_VIEWPORT_SIZE = MStringId("u_f2ViewportSize");
MStringId MShaderPropertyName::FRAME_Z_NEAR_FAR = MStringId("u_matZNearFar");
MStringId MShaderPropertyName::FRAME_TIME_DELTA = MStringId("u_fDelta");
MStringId MShaderPropertyName::FRAME_GAME_TIME = MStringId("u_fGameTime");

MStringId MShaderPropertyName::LIGHT_ENVIRONMENT_MAP_ENABLE = MStringId("u_bEnvironmentMapEnabled");
MStringId MShaderPropertyName::LIGHT_DIRECTION_LIGHT_ENABLE = MStringId("u_bDirectionLightEnabled");
MStringId MShaderPropertyName::LIGHT_DIRECTION_STRUCT_NAME = MStringId("u_xDirectionalLight");
MStringId MShaderPropertyName::LIGHT_DIRECTION_LIGHT_DIR = MStringId("f3LightDir");
MStringId MShaderPropertyName::LIGHT_INTENSITY = MStringId("f3Intensity");
MStringId MShaderPropertyName::LIGHT_DIRECTION_LIGHT_SIZE = MStringId("fLightSize");

MStringId MShaderPropertyName::LIGHT_POINT_ARRAY_NAME = MStringId("u_vPointLights");
MStringId MShaderPropertyName::LIGHT_POINT_POSITION = MStringId("f3WorldPosition");
MStringId MShaderPropertyName::LIGHT_POINT_CONSTANT = MStringId("fConstant");
MStringId MShaderPropertyName::LIGHT_POINT_LINEAR = MStringId("fLinear");
MStringId MShaderPropertyName::LIGHT_POINT_QUADRATIC = MStringId("fQuadratic");
MStringId MShaderPropertyName::LIGHT_POINT_COUNT = MStringId("u_nValidPointLightsNumber");

MStringId MShaderPropertyName::SHADOW_LIGHT_PROJ_MATRIX = MStringId("u_vLightProjectionMatrix");
MStringId MShaderPropertyName::SHADOW_LIGHT_CASCADE_SPLIT = MStringId("u_vCascadeSplits");
MStringId MShaderPropertyName::SHADOW_GENERATE_CBUFFER_MATRIX_NAME = MStringId("cbShadowMatrix");

MStringId MShaderPropertyName::VOXEL_MAP_SETTING = MStringId("voxelMapSetting");
MStringId MShaderPropertyName::VOXEL_MAP_ORIGIN = MStringId("f3VoxelOrigin");
MStringId MShaderPropertyName::VOXEL_MAP_RESOLUTION = MStringId("nResolution");
MStringId MShaderPropertyName::VOXEL_MAP_VIEWPORT_SIZE = MStringId("nViewportSize");
MStringId MShaderPropertyName::VOXEL_MAP_STEP_SIZE = MStringId("fVoxelSize");
MStringId MShaderPropertyName::VOXEL_MAP_CLIPMAP_ARRAY = MStringId("vClipmap");
MStringId MShaderPropertyName::VOXEL_MAP_CLIPMAP_INDEX = MStringId("nClipmapIdx");
MStringId MShaderPropertyName::VOXELIZER_CAMERA_PROJ_MATRIX = MStringId("u_m4VoxelizerCamProj");
MStringId MShaderPropertyName::VOXELIZER_CBUFFER_VOXEL_MAP_NAME = MStringId("cbVoxelMap");
MStringId MShaderPropertyName::VOXELIZER_CUBE_MESH_INDEX = MStringId("cubeMeshIndex");
MStringId MShaderPropertyName::VOXELIZER_CUBE_MESH_COUNT = MStringId("cubeMeshCount");
MStringId MShaderPropertyName::VOXELIZER_VOXEL_TEXTURE_NAME = MStringId("u_texVoxelMap");
MStringId MShaderPropertyName::VOXELIZER_VOXEL_TABLE_NAME = MStringId("rVoxelTable");

MStringId MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_0 = MStringId("u_texSubpassInput0");
MStringId MShaderPropertyName::TRANSPARENT_TEXTURE_INPUT_1 = MStringId("u_texSubpassInput1");

MStringId MShaderPropertyName::CBUFFER_MESH_MATRIX = MStringId("u_meshMatrix");
MStringId MShaderPropertyName::MESH_LOCAL_MATRIX = MStringId("u_meshMatrix");
MStringId MShaderPropertyName::MESH_WORLD_MATRIX = MStringId("u_matWorld");
MStringId MShaderPropertyName::MESH_NORMAL_MATRIX = MStringId("u_matNormal");
MStringId MShaderPropertyName::MESH_INSTANCE_INDEX = MStringId("u_meshIdx");
MStringId MShaderPropertyName::MESH_INSTANCE_BEGIN_INDEX = MStringId("u_meshInstanceBeginIndex");

MStringId MShaderPropertyName::MATERIAL_CBUFFER_NAME = MStringId("cbMaterial");
MStringId MShaderPropertyName::MATERIAL_STRUCT_NAME = MStringId("u_xMaterial");
MStringId MShaderPropertyName::MATERIAL_METALLIC = MStringId("fMetallic");
MStringId MShaderPropertyName::MATERIAL_ROUGHNESS = MStringId("fRoughness");
MStringId MShaderPropertyName::MATERIAL_ALBEDO = MStringId("f4Albedo");
MStringId MShaderPropertyName::MATERIAL_AMBIENT = MStringId("f3Ambient");
MStringId MShaderPropertyName::MATERIAL_DIFFUSE = MStringId("f3Diffuse");
MStringId MShaderPropertyName::MATERIAL_SPECULAR = MStringId("f3Specular");
MStringId MShaderPropertyName::MATERIAL_SHININESS = MStringId("fShininess");
MStringId MShaderPropertyName::MATERIAL_NORMAL_TEXTURE_ENABLE = MStringId("bUseNormalTex");
MStringId MShaderPropertyName::MATERIAL_ALPHA_FACTOR = MStringId("fAlphaFactor");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_DIFFUSE = MStringId("u_texDiffuse");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_SPECULAR = MStringId("u_texSpecular");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_ALBEDO = MStringId("u_mat_texAlbedo");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_NORMAL = MStringId("u_texNormal");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_METALLIC = MStringId("u_mat_texMetallic");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_ROUGHNESS = MStringId("u_mat_texRoughness");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_AMBIENTOCC = MStringId("u_mat_texAmbientOcc");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_HEIGHT = MStringId("u_mat_texHeight");
MStringId MShaderPropertyName::MATERIAL_TEXTURE_EMISSION = MStringId("u_mat_texEmission");

MStringId MShaderPropertyName::GBUFFER_TEXTURE_ALBEDO_METALLIC = MStringId("u_mat_f3Albedo_fMetallic");
MStringId MShaderPropertyName::GBUFFER_TEXTURE_NORMAL_ROUGHNESS = MStringId("u_mat_f3Normal_fRoughness");
MStringId MShaderPropertyName::GBUFFER_TEXTURE_POSITION_AMBIENTOCC = MStringId("u_mat_f3Position_fAmbientOcc");
MStringId MShaderPropertyName::GBUFFER_TEXTURE_DEPTH_MAP = MStringId("u_mat_DepthMap");

MStringId MShaderPropertyName::CULLING_INSTANCE_DATA = MStringId("instances");
MStringId MShaderPropertyName::CULLING_OUTPUT_DRAW_DATA = MStringId("indirectDraws");
MStringId MShaderPropertyName::CULLING_OUTPUT_DRAW_COUNT = MStringId("uboOut");
MStringId MShaderPropertyName::CULLING_CBUFFER_FRAME_DATA = MStringId("ubo");
MStringId MShaderPropertyName::CULLING_CAMERA_POSITION = MStringId("cameraPos");
MStringId MShaderPropertyName::CULLING_FRUSTUM_PLANES = MStringId("frustumPlanes");

MStringId MShaderPropertyName::ENVIRONMENT_TEXTURE_SKYBOX = MStringId("u_texSkyBox");
MStringId MShaderPropertyName::ENVIRONMENT_IBL_MVP_MATRIX = MStringId("u_ModelViewProj");
MStringId MShaderPropertyName::ENVIRONMENT_IBL_ROUGHNESS = MStringId("u_roughness");

MStringId MShaderPropertyName::POSTPROCESS_SCREEN_TEXTURE = MStringId("u_texScreenTexture");

MStringId MShaderPropertyName::IMGUI_SCALE = MStringId("u_f2Scale");
MStringId MShaderPropertyName::IMGUI_TRANSLATE = MStringId("u_f2Translate");
MStringId MShaderPropertyName::IMGUI_IMAGE_TYPE = MStringId("u_nImageType");
MStringId MShaderPropertyName::IMGUI_IMAGE_ARRAY = MStringId("u_nImageArray");
MStringId MShaderPropertyName::IMGUI_IMAGE_INDEX = MStringId("u_nImageIndex");

