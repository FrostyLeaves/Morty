#include "Render/MRenderGlobal.h"

const char* MRenderGlobal::SHADER_SKELETON_ENABLE = "SKELETON_ENABLE";

const char* MRenderGlobal::SUFFIX_VERTEX_SHADER = "mvs";

const char* MRenderGlobal::SUFFIX_PIXEL_SHADER = "mps";

const char* MRenderGlobal::SUFFIX_COMPUTE_SHADER = "mcs";

const char* MRenderGlobal::SUFFIX_GEOMETRY_SHADER = "mgs";

const bool MRenderGlobal::GBUFFER_UNIFIED_FORMAT = false;

const int MRenderGlobal::SHADOW_TEXTURE_SIZE = 2048;

const int MRenderGlobal::VOXEL_TABLE_SIZE = 64;

const char* MRenderGlobal::DRAW_MESH_INSTANCING_UNIFORM = "DRAW_MESH_INSTANCING_UNIFORM";
const char* MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE = "DRAW_MESH_INSTANCING_STORAGE";

const size_t MRenderGlobal::MERGE_INSTANCING_MAX_NUM = 128;
const size_t MRenderGlobal::MERGE_INSTANCING_CLUSTER_MAX_NUM = 1024;

const size_t MRenderGlobal::MESH_TRANSFORM_IN_UNIFORM_MAX_NUM = 128;