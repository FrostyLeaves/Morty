/**
 * @File         MGPUCullingRenderWork
 * 
 * @Created      2022-12-29 16:58:45
 *
 * @Author       Pobrecito
**/

#ifndef _M_MGPU_CULLING_RENDER_WORK_H_
#define _M_MGPU_CULLING_RENDER_WORK_H_
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "MRenderInfo.h"
#include "Render/MBuffer.h"
#include "Render/MRenderPass.h"
#include "MShadowMapShaderParamSet.h"
#include "Render/MVertex.h"

class MTaskNode;
class MIRenderCommand;
class MComputeDispatcher;
class MORTY_API MGPUCullingRenderWork : public MObject
{
	MORTY_CLASS(MGPUCullingRenderWork)
public:
	MGPUCullingRenderWork();
    virtual ~MGPUCullingRenderWork();

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

public:

	void CollectCullingGroup(MRenderInfo& info);
	void UpdateCameraFrustum(MRenderInfo& info);
	void DispatchCullingJob(MRenderInfo& info);

	void ClearCullingGroup();

	const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const { return m_vCullingInstanceGroup; }

	const MBuffer* GetDrawIndirectBuffer() const { return &m_cullingIndirectDrawBuffer; }

private:

	MShaderConstantParam* m_pIndirectTransformParam;
	MBuffer m_cullingInstanceBuffer;
	MBuffer m_cullingIndirectDrawBuffer;
	MBuffer m_cullingDrawCallBuffer;
	MComputeDispatcher* m_pCullingComputeDispatcher;
	std::vector<MMaterialCullingGroup> m_vCullingInstanceGroup;
	std::vector<MMergeInstanceCullData> m_vInstanceCullData;
};


#endif
