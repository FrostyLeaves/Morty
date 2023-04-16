/**
 * @File         MGPUCullingRenderWork
 * 
 * @Created      2022-12-29 16:58:45
 *
 * @Author       DoubleYe
**/

#ifndef _M_MGPU_CULLING_RENDER_WORK_H_
#define _M_MGPU_CULLING_RENDER_WORK_H_
#include "MRenderWork.h"
#include "Utility/MGlobal.h"
#include "Object/MObject.h"

#include "Render/MBuffer.h"
#include "Render/MRenderPass.h"
#include "Render/MVertex.h"
#include "RenderProgram/MRenderInfo.h"

class MTaskNode;
class MIRenderCommand;
class MComputeDispatcher;
class MORTY_API MGPUCullingRenderWork : public IRenderWork
{
	MORTY_CLASS(MGPUCullingRenderWork)
public:

	void Initialize(MEngine* pEngine) override;
	void Release(MEngine* pEngine) override;
	void Resize(Vector2 size) override;

	MEngine* GetEngine() const { return m_pEngine; }

public:

	void CollectCullingGroup(MRenderInfo& info);
	void UpdateCameraFrustum(MRenderInfo& info);
	void DispatchCullingJob(MRenderInfo& info);

	void ClearCullingGroup();

	const std::vector<MMaterialCullingGroup>& GetCullingInstanceGroup() const { return m_vCullingInstanceGroup; }

	const MBuffer* GetDrawIndirectBuffer() const { return &m_cullingIndirectDrawBuffer; }
//	const MBuffer* GetDrawShadowIndirectBuffer() const { return &m_cullingIndirectDrawShadowBuffer; }

private:

	MEngine* m_pEngine = nullptr;

	MShaderConstantParam* m_pIndirectTransformParam = nullptr;
	MBuffer m_cullingInstanceBuffer;
	MBuffer m_cullingIndirectDrawBuffer;
//	MBuffer m_cullingIndirectDrawShadowBuffer;
	MBuffer m_cullingDrawCallBuffer;
	MComputeDispatcher* m_pCullingComputeDispatcher = nullptr;
	std::vector<MMaterialCullingGroup> m_vCullingInstanceGroup;
	std::vector<MMergeInstanceCullData> m_vInstanceCullData;
};


#endif
