#pragma once

#include "Utility/MGlobal.h"
#include "Variant/MVariant.h"
#include "Material/MMaterial.h"
#include "Basic/MCameraFrustum.h"
#include "RenderProgram/MRenderInfo.h"

class MIMesh;
class MScene;
class MEngine;
class MMaterial;
class MComponent;
class MShaderPropertyBlock;
class MRenderMeshComponent;
struct MShaderConstantParam;



class MORTY_API MShadowMapUtil
{
public:

	static std::array<MCascadedShadowSceneData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> CascadedSplitCameraFrustum(MViewport* pViewport);

	static std::array<MCascadedShadowRenderData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM> CalculateRenderData(
		MViewport* pViewport, MEntity* pCameraEntity,
		const std::array<MCascadedShadowSceneData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>& vCascadedData,
		const std::array<MBoundsAABB, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>& vCascadedPscBounds);

};
