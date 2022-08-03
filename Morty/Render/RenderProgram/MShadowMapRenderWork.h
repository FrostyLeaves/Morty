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

class MTaskNode;
class MIRenderCommand;
class MORTY_API MShadowMapRenderWork : public MObject
{
	MORTY_CLASS(MShadowMapRenderWork)
public:
    MShadowMapRenderWork();
    virtual ~MShadowMapRenderWork();

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

public:

	void CollectShadowMesh(MRenderInfo& info);

	void RenderShadow(MRenderInfo& info);

	void UpdateShadowParams(MRenderInfo& info);

public:

	MTexture* GetShadowMap();

protected:

	void DrawShadowMesh(MRenderInfo& info, MIRenderCommand* pCommand);



	Matrix4 GetLightInverseProjection_MinBoundsAABB(MRenderInfo& info, const MBoundsAABB& cGenerateShadowAABB, float fZNear, float fZFar);

	Matrix4 GetLightInverseProjection_MaxBoundsSphere(MRenderInfo& info, const MBoundsAABB& cGenerateShadowAABB, float fZNear, float fZFar);



	void CalculateFrustumForCascadesShadowMap(MRenderInfo& info);

private:


	std::shared_ptr<MMaterial> m_pShadowStaticMaterial;
	std::shared_ptr<MMaterial> m_pShadowSkeletonMaterial;

	MRenderPass m_renderPass;
	MShadowMapShaderParamSet m_shadowParamSet;

};


#endif
