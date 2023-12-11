#include "Render/MRenderGlobal.h"

const char* MRenderGlobal::SUFFIX_VERTEX_SHADER = "mvs";

const char* MRenderGlobal::SUFFIX_PIXEL_SHADER = "mps";

const char* MRenderGlobal::SUFFIX_COMPUTE_SHADER = "mcs";

const char* MRenderGlobal::SUFFIX_GEOMETRY_SHADER = "mgs";

MStringId MRenderGlobal::SHADER_SKELETON_ENABLE = MStringId("SKELETON_ENABLE");
MStringId MRenderGlobal::DRAW_MESH_INSTANCING_NONE = MStringId("DRAW_MESH_INSTANCING_NONE");
MStringId MRenderGlobal::DRAW_MESH_INSTANCING_UNIFORM = MStringId("DRAW_MESH_INSTANCING_UNIFORM");
MStringId MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE = MStringId("DRAW_MESH_INSTANCING_STORAGE");
MStringId MRenderGlobal::VOXELIZER_CONSERVATIVE_RASTERIZATION = MStringId("VOXELIZER_CONSERVATIVE_RASTERIZATION");
MStringId MRenderGlobal::MEN_TRANSPARENT = MStringId("MEN_TRANSPARENT");
MString MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG = "1";
MString MRenderGlobal::SHADER_DEFINE_DISABLE_FLAG = "0";

MStringId MRenderGlobal::TASK_RENDER_MESH_MANAGER_UPDATE = MStringId("RenderMeshManagerUpdate");
MStringId MRenderGlobal::TASK_ANIMATION_MANAGER_UPDATE = MStringId("AnimationManagerUpdate");
MStringId MRenderGlobal::TASK_SHADOWMAP_MANAGER_UPDATE = MStringId("ShadowMeshManagerUpdate");
MStringId MRenderGlobal::TASK_UPLOAD_MESH_UPDATE = MStringId("Upload Mesh Buffer");
MStringId MRenderGlobal::TASK_RENDER_MODULE_UPDATE = MStringId("Render_Update");

MStringId MRenderGlobal::POSTPROCESS_FINAL_NODE = MStringId("PostProcessFinalNode");
MStringId MRenderGlobal::POSTPROCESS_EDGE_DETECTION = MStringId("PostProcessEdgeDetection");
