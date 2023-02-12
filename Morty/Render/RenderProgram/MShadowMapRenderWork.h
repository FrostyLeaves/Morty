/**
 * @File         MShadowMapRenderWork
 * 
 * @Created      2021-08-16 10:37:01
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSHADOWMAPRENDERWORK_H_
#define _M_MSHADOWMAPRENDERWORK_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "MRenderInfo.h"
#include "Render/MRenderPass.h"
#include "MShadowMapShaderParamSet.h"
#include "Basic/MCameraFrustum.h"

class MGPUCullingRenderWork;
class MTaskNode;
class MIRenderCommand;
class MORTY_API MShadowMapRenderWork : public MObject
{
	MORTY_CLASS(MShadowMapRenderWork)

public:
	struct CascadedData
	{
		bool bGenerateShadow = false;
		Vector3 v3ShadowMin = Vector3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
		Vector3 v3ShadowMax = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		//Potential Shadow Caster.
		MBoundsAABB cPcsBounds;

		//range: 0.0 - 1.0f
		float fCascadeSplit = 0.0f;
		float fNearZ = 0.0f;
		float fFarZ = 0.0f;

		MCameraFrustum cCameraFrustum;

	};

public:
    MShadowMapRenderWork();
    virtual ~MShadowMapRenderWork();

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

public:

	void CollectShadowMesh(MRenderInfo& info);

	void RenderShadow(MRenderInfo& info, MGPUCullingRenderWork* pGPUCullingRenderWork);

	void UpdateShadowParams(MRenderInfo& info);

public:

	std::shared_ptr<MTexture> GetShadowMap();

protected:

	void DrawShadowMesh(MRenderInfo& info, MIRenderCommand* pCommand);

	void DrawShadowMesh(MGPUCullingRenderWork* pGPUCullingRenderWork, MIRenderCommand* pCommand);


	Matrix4 GetLightInverseProjection_MinBoundsAABB(MRenderInfo& info, const MBoundsAABB& cGenerateShadowAABB, float fZNear, float fZFar);

	Matrix4 GetLightInverseProjection_MaxBoundsSphere(MRenderInfo& info, const MBoundsAABB& cGenerateShadowAABB, float fZNear, float fZFar);



	void CalculateFrustumForCascadesShadowMap(MRenderInfo& info, const std::array<CascadedData, MRenderGlobal::CASCADED_SHADOW_MAP_NUM>& vCascadedData);

private:


	std::shared_ptr<MMaterial> m_pShadowStaticMaterial = nullptr;
	std::shared_ptr<MMaterial> m_pShadowSkeletonMaterial = nullptr;
	std::shared_ptr<MMaterial> m_pShadowGPUDrivenMaterial = nullptr;

	MRenderPass m_renderPass;
	MShadowMapShaderPropertyBlock m_shadowPropertyBlock;

};


#endif
