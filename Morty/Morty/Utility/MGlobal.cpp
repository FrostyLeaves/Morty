#include "MGlobal.h"

const int MGlobal::M_INVALID_INDEX = -1;

const char* MGlobal::SUFFIX_VERTEX_SHADER = "mvs";
const char* MGlobal::SUFFIX_PIXEL_SHADER = "mps";
const char* MGlobal::SUFFIX_MATERIAL = "matl";
const char* MGlobal::SUFFIX_SKELETON = "mske";
const char* MGlobal::SUFFIX_MESH = "mesh";
const char* MGlobal::SUFFIX_NODE = "mnode";
const char* MGlobal::SUFFIX_SKELANIM = "mseq";

const char* MGlobal::DEFAULT_MATERIAL_MODEL_STATIC_MESH = "DefMatModelStaticMesh";
const char* MGlobal::DEFAULT_MATERIAL_MODEL_SKELETON_MESH = "DefMatModelSkeletonMesh";

const char* MGlobal::DEFAULT_MATERIAL_MODEL_STATIC_MESH_PBR = "DefMatModelStaticMeshPBR";
const char* MGlobal::DEFAULT_MATERIAL_MODEL_SKELETON_MESH_PBR = "DefMatModelSkeletonMeshPBR";

const char* MGlobal::DEFAULT_MATERIAL_DRAW2D = "DefMatDraw2D";
const char* MGlobal::DEFAULT_MATERIAL_DRAW3D = "DefMatDraw3D";
const char* MGlobal::DEFAULT_MATERIAL_SHADOW_STATIC = "DefMatShadow";
const char* MGlobal::DEFAULT_MATERIAL_SHADOW_SKELETON = "DefMatShadowAnim";
const char* MGlobal::DEFAULT_MATERIAL_DEPTH_PEEL_BLEND = "DefMatDepthPeelBlend";
const char* MGlobal::DEFAULT_MATERIAL_DEPTH_PEEL_FILL = "DefMatDepthPeelFill";
const char* MGlobal::DEFAULT_MESH_SCREEN_DRAW = "DefMeshScreenDraw";

const char* MGlobal::DEFAULT_TEXTURE_WHITE = "DefTexWhite";
const char* MGlobal::DEFAULT_TEXTURE_BLACK = "DefTexBlack";
const char* MGlobal::DEFAULT_TEXTURE_NORMALMAP = "DefTexNormalMap";

const char* MGlobal::SHADER_PARAM_NAME_DIFFUSE = "U_mat_texDiffuse";
const char* MGlobal::SHADER_PARAM_NAME_NORMAL = "U_mat_texNormal";

const uint32_t MGlobal::MSHADOW_TEXTURE_SIZE = 2048;

const uint32_t MGlobal::MPOINT_LIGHT_MAX_NUMBER = 8;
const uint32_t MGlobal::MPOINT_LIGHT_PIXEL_NUMBER = 4;

const uint32_t MGlobal::MSPOT_LIGHT_MAX_NUMBER = 8;
const uint32_t MGlobal::MSPOT_LIGHT_PIXEL_NUMBER = 4;

const bool MGlobal::MCALC_NORMAL_IN_VS = false;


const uint32_t MGlobal::MMESH_LOD_LEVEL_RANGE = 10;


const char* MGlobal::MATERIAL_MACRO_SKELETON_ENABLE = "SKELETON_ENABLE";


const uint32_t MGlobal::SHADER_PARAM_SET_MATERIAL = 0;
const uint32_t MGlobal::SHADER_PARAM_SET_FRAME = 1;
const uint32_t MGlobal::SHADER_PARAM_SET_MESH = 2;
const uint32_t MGlobal::SHADER_PARAM_SET_SKELETON = 3;

