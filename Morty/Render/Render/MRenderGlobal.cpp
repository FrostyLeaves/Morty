﻿#include "Render/MRenderGlobal.h"

const char* MRenderGlobal::SHADER_SKELETON_ENABLE = "SKELETON_ENABLE";

const char* MRenderGlobal::SUFFIX_VERTEX_SHADER = "mvs";

const char* MRenderGlobal::SUFFIX_PIXEL_SHADER = "mps";

const bool MRenderGlobal::GBUFFER_UNIFIED_FORMAT = false;

const int MRenderGlobal::SHADOW_TEXTURE_SIZE = 4096;

const char* MRenderGlobal::DRAW_MESH_INSTANCING_UNIFORM = "DRAW_MESH_INSTANCING_UNIFORM";
const char* MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE = "DRAW_MESH_INSTANCING_STORAGE";

const size_t MRenderGlobal::MERGE_INSTANCING_MAX_NUM = 128;
const size_t MRenderGlobal::MERGE_INSTANCING_CLUSTER_MAX_NUM = 1024;