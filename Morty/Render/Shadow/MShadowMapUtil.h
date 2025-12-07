#pragma once

#include "Utility/MGlobal.h"
#include "Basic/MCameraFrustum.h"
#include "Material/MMaterial.h"
#include "Render/MRenderInfo.h"
#include "Variant/MVariant.h"

namespace morty
{

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MShaderParameterSet;
class MRenderMeshComponent;
struct MShaderUniformParam;


template<typename TYPE> using MCascadedArray = std::array<TYPE, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>;

class MORTY_API MShadowMapUtil
{
public:
    static MCascadedArray<MCascadedSplitData>        CascadedSplitCameraFrustum(MViewport* viewport);

    static MCascadedArray<MCascadedShadowRenderData> CalculateRenderData(
            MViewport*                                viewport,
            MEntity*                                  pCameraEntity,
            const MCascadedArray<MCascadedSplitData>& vCascadedData,
            const MCascadedArray<MBoundsSphere>&      vCascadedPsrBounds,
            const MCascadedArray<MBoundsAABB>&        vCascadedPscBounds
    );


    static MCascadedArray<MBoundsSphere>
    GetCameraFrustumBounds(MViewport* viewport, const MCascadedArray<MCascadedSplitData>& vCascadedSplitData);

    static MCascadedArray<std::unique_ptr<class IRenderableFilter>>
    GetCameraFrustumCullingFilter(MViewport* viewport, const MCascadedArray<MCascadedSplitData>& vCascadedSplitData);

    static MCascadedArray<MBoundsSphere>
    GetVoxelMapBounds(MViewport* viewport, const MCascadedArray<MCascadedSplitData>& vCascadedSplitData);

    static MCascadedArray<std::unique_ptr<class IRenderableFilter>>
    GetBoundsCullingFilter(MViewport* viewport, const MCascadedArray<MBoundsSphere>& vBoundsSphere);
};

}// namespace morty