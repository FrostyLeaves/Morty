#include "MGlobal.h"

const int M_INVALID_INDEX = -1;

const char* SUFFIX_VERTEX_SHADER = "mvs";
const char* SUFFIX_PIXEL_SHADER = "mps";
const char* SUFFIX_MATERIAL = "matl";
const char* SUFFIX_SKELETON = "mske";
const char* SUFFIX_MESH = "mesh";
const char* SUFFIX_MODEL = "model";
const char* SUFFIX_SKELANIM = "mseq";

const char* DEFAULT_MATERIAL_MODEL_STATIC_MESH = "DefMatModelStaticMesh";
const char* DEFAULT_MATERIAL_MODEL_SKELETON_MESH = "DefMatModelSkeletonMesh";
const char* DEFAULT_MATERIAL_DRAW2D = "DefMatDraw2D";
const char* DEFAULT_MATERIAL_DRAW3D = "DefMatDraw3D";
const char* DEFAULT_MATERIAL_SKYBOX = "DefMatSkyBox";
const char* DEFAULT_MATERIAL_SHADOW_STATIC = "DefMatShadow";
const char* DEFAULT_MATERIAL_SHADOW_SKELETON = "DefMatShadowAnim";
const char* DEFAULT_MATERIAL_DEPTH_PEEL_BLEND = "DefMatDepthPeelBlend";
const char* DEFAULT_MATERIAL_DEPTH_PEEL_FILL = "DefMatDepthPeelFill";

const char* DEFAULT_TEXTURE_WHITE = "DefTexWhite";
const char* DEFAULT_TEXTURE_BLACK = "DefTexBlack";
const char* DEFAULT_TEXTURE_NORMALMAP = "DefTexNormalMap";

const char* SHADER_PARAM_NAME_DIFFUSE = "U_mat_texDiffuse";
const char* SHADER_PARAM_NAME_NORMAL = "U_mat_texNormal";

const uint32_t MSHADOW_TEXTURE_SIZE = 2048;

const uint32_t MPOINT_LIGHT_MAX_NUMBER = 8;
const uint32_t MPOINT_LIGHT_PIXEL_NUMBER = 4;

const uint32_t MSPOT_LIGHT_MAX_NUMBER = 8;
const uint32_t MSPOT_LIGHT_PIXEL_NUMBER = 4;

const bool MCALC_NORMAL_IN_VS = false;


const uint32_t MMESH_LOD_LEVEL_RANGE = 10;


const char* MATERIAL_MACRO_SKELETON_ENABLE = "SKELETON_ENABLE";