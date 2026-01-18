/**
 * @File         MIDevice
 * 
 * @Created      2021-7-7 14:20:55
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MShaderPropertyName.h"
#include "Utility/MStringId.h"

#if RENDER_GRAPHICS == MORTY_VULKAN
#include "RHI/Vulkan/MVulkanWrapper.h"
#endif

#include "Flatbuffer/MRenderGlobal_generated.h"

#define GPU_CULLING_ENABLE  (false)
#define MORTY_VXGI_ENABLE   (false)
#define VRS_OPTIMIZE_ENABLE (false)
#define SHADER_STRUCT struct alignas(16) [[clang::annotate("ShaderStruct")]]

namespace morty
{
class MTexture;

class MORTY_API MRenderGlobal
{
public:
    static constexpr int      SHADER_PARAM_SET_MATERIAL = 0;
    static constexpr int      SHADER_PARAM_SET_FRAME    = 1;
    static constexpr int      SHADER_PARAM_SET_MESH     = 2;
    static constexpr int      SHADER_PARAM_SET_OTHER    = 3;
    static constexpr int      SHADER_PARAM_SET_NUM      = 4;

    static constexpr int      BONES_PER_VERTEX    = 4;
    static constexpr int      BONES_MAX_NUMBER    = 128;
    static constexpr int      SHADOW_TEXTURE_SIZE = 4096;
    static constexpr uint32_t VOXEL_TABLE_SIZE    = 64;
    static constexpr uint32_t VOXEL_VIEWPORT_SIZE = VOXEL_TABLE_SIZE * 2;

    static constexpr int      POINT_LIGHT_MAX_NUMBER   = 8;
    static constexpr int      POINT_LIGHT_PIXEL_NUMBER = 8;
    static constexpr int      SPOT_LIGHT_MAX_NUMBER    = 8;
    static constexpr int      SPOT_LIGHT_PIXEL_NUMBER  = 8;

    static constexpr int      MESH_LOD_LEVEL_RANGE = 3;

    static const char*        SUFFIX_VERTEX_SHADER;
    static const char*        SUFFIX_PIXEL_SHADER;
    static const char*        SUFFIX_COMPUTE_SHADER;
    static const char*        SUFFIX_GEOMETRY_SHADER;
    static const char*        SUFFIX_SLANG_SHADER;
    static const char*        SUFFIX_HLSL_SHADER;

    static MStringId          DEFAULT_PASS_NAME;
    static MStringId          DEFAULT_VERTEX_ENTRY;
    static MStringId          DEFAULT_PIXEL_ENTRY;
    static MStringId          DEFAULT_COMPUTE_ENTRY;
    static MStringId          DEFAULT_GEOMETRY_ENTRY;

    static constexpr int      CASCADED_SHADOW_MAP_NUM  = 4;
    static constexpr int      VOXEL_GI_CLIP_MAP_NUM    = 6;
    static constexpr float    VOXEL_BASIC_VOXEL_SIZE   = 1.0f;
    static constexpr int      VOXEL_DIFFUSE_CONE_COUNT = 16;

    static MStringId          SHADER_SKELETON_ENABLE;
    static MStringId          DRAW_MESH_INSTANCING_NONE;
    static MStringId          DRAW_MESH_INSTANCING_UNIFORM;
    static MStringId          DRAW_MESH_INSTANCING_STORAGE;
    static MStringId          VOXELIZER_CONSERVATIVE_RASTERIZATION;
    static MStringId          MEN_TRANSPARENT;
    static MString            SHADER_DEFINE_ENABLE_FLAG;
    static MString            SHADER_DEFINE_DISABLE_FLAG;


    static constexpr size_t   MESH_TRANSFORM_IN_UNIFORM_MAX_NUM = 128;

    static MStringId          TASK_RENDER_MESH_MANAGER_UPDATE;
    static MStringId          TASK_ANIMATION_MANAGER_UPDATE;
    static MStringId          TASK_SHADOWMAP_MANAGER_UPDATE;
    static MStringId          TASK_UPLOAD_MESH_UPDATE;
    static MStringId          TASK_RENDER_MODULE_UPDATE;

    static MStringId          POSTPROCESS_FINAL_NODE;
    static MStringId          POSTPROCESS_EDGE_DETECTION;

    static constexpr int      THREAD_ID_SUBMIT = 3;
};

using METextureType        = fbs::METextureType;
using METextureFormat      = fbs::METextureFormat;
using MEMipmapDataType     = fbs::MEMipmapDataType;
using MECullMode           = fbs::MECullMode;
using MEDepthFunc          = fbs::MEDepthFunc;
using MEShaderType         = fbs::MEShaderType;
using MEBlendFactor        = fbs::MEBlendFactor;
using MEBlendOp            = fbs::MEBlendOp;
using MEStencilOp          = fbs::MEStencilOp;
using MMeshInstanceKey     = size_t;
using MSkeletonInstanceKey = size_t;
using MTexturePtr          = std::shared_ptr<MTexture>;
using MTextureArray        = std::vector<MTexturePtr>;
using MEntryNames          = std::array<MStringId, static_cast<int>(MEShaderType::TOTAL_NUM)>;
using MIndicesType         = uint32_t;
using METextureWriteUsage  = uint32_t;
using METextureReadUsage   = uint32_t;

enum class MECameraType
{
    EPerspective  = 1,
    EOrthographic = 2,
};

enum class MEDeviceFeature
{
    EConservativeRasterization,
    EHLSLFunctionality,
    EVariableRateShading,
    EDeviceFault,
};

enum class MEBufferBarrierStage
{
    EUnknow = 0,
    EComputeShaderWrite,
    EComputeShaderRead,
    EPixelShaderWrite,
    EPixelShaderRead,
    EDrawIndirectRead,
    EShadingRateRead,
};

enum class MEShadingRateCombinerOp
{
    Keep = 0,
    Replace,
    Min,
    Max,
    Mul,
};

enum class MRenderNodeOutputType
{
    RenderTarget,
    Data,
};

struct MShadingRateType {
    static constexpr MByte Rate_1x1 = 0;
    static constexpr MByte Rate_1X2 = 1;
    static constexpr MByte Rate_2X1 = 4;
    static constexpr MByte Rate_2X2 = 5;
    static constexpr MByte Rate_2X4 = 6;
    static constexpr MByte Rate_4X2 = 9;
    static constexpr MByte Rate_4X4 = 10;
};

struct MShaderUsageMask {
    static constexpr uint8_t Vertex   = 1 << static_cast<uint8_t>(MEShaderType::EVertex);
    static constexpr uint8_t Pixel    = 1 << static_cast<uint8_t>(MEShaderType::EPixel);
    static constexpr uint8_t Compute  = 1 << static_cast<uint8_t>(MEShaderType::ECompute);
    static constexpr uint8_t Geometry = 1 << static_cast<uint8_t>(MEShaderType::EGeometry);
};

template<class VERTEX_TYPE> inline size_t             AttributeProtectMask() { return 0; }
template<class VERTEX_TYPE> inline std::vector<float> SimplifyWeight() { return {}; }


}// namespace morty