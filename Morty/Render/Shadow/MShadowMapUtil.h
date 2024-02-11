#pragma once

#include "Utility/MGlobal.h"
#include "Variant/MVariant.h"
#include "Material/MMaterial.h"
#include "Basic/MCameraFrustum.h"
#include "RenderProgram/MRenderInfo.h"

MORTY_SPACE_BEGIN

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MShaderPropertyBlock;
class MRenderMeshComponent;
struct MShaderConstantParam;


template<typename TYPE>
using MCascadedArray = std::array<TYPE, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>;

class MORTY_API MShadowMapUtil
{
public:

	static MCascadedArray<MCascadedSplitData> CascadedSplitCameraFrustum(MViewport* pViewport);

	static MCascadedArray<MCascadedShadowRenderData> CalculateRenderData(
		MViewport* pViewport, MEntity* pCameraEntity,
		const MCascadedArray<MCascadedSplitData>& vCascadedData,
		const MCascadedArray<MBoundsSphere>& vCascadedPsrBounds,
		const MCascadedArray<MBoundsAABB>& vCascadedPscBounds);


	static MCascadedArray<MBoundsSphere> GetCameraFrustumBounds(MViewport* pViewport,
		const MCascadedArray<MCascadedSplitData>& vCascadedSplitData);

	static MCascadedArray<std::unique_ptr<class IRenderableFilter>> GetCameraFrustumCullingFilter(MViewport* pViewport,
		const MCascadedArray<MCascadedSplitData>& vCascadedSplitData);

	static MCascadedArray<MBoundsSphere> GetVoxelMapBounds(MViewport* pViewport, const MCascadedArray<MCascadedSplitData>& vCascadedSplitData);

	static MCascadedArray<std::unique_ptr<class IRenderableFilter>> GetBoundsCullingFilter(MViewport* pViewport, const MCascadedArray<MBoundsSphere>& vBoundsSphere);
};

MORTY_SPACE_END