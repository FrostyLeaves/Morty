/**
 * @File         MForwardRenderWork
 * 
 * @Created      2020-11-29 11:53:33
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFORWARDRENDERWORK_H_
#define _M_MFORWARDRENDERWORK_H_
#include "MGlobal.h"
#include "MObject.h"

#include "Type/MColor.h"
#include "MRenderPass.h"

#include "MForwardRenderProgram.h"
#include "MForwardRenderShaderParamSet.h"

class MRenderGraphNode;
class MModelComponent;
class MRenderableMeshComponent;
class MORTY_API MForwardRenderWork : public MObject
{
public:
	M_OBJECT(MForwardRenderWork);
    MForwardRenderWork();
    virtual ~MForwardRenderWork();

public:
	void Initialize(MIRenderProgram* pRenderProgram);
	void Release();

	void InitializeShaderParamSet();
	void ReleaseShaderParamSet();

	void InitializeRenderGraph();
	void ReleaseRenderGraph();

	virtual void OnDelete() override;

	void Render(MRenderGraphNode* pGraphNode);

protected:

	void DrawNormalMesh(MRenderInfo& info);

	void DrawModelBoundingBox(MRenderInfo& info);
	void DrawSkyBox(MRenderInfo& info);
	void DrawPainter(MRenderInfo& info);
	void DrawBoundingBox(MRenderInfo& info, MModelComponent* pModelComponent);
	void DrawBoundingSphere(MRenderInfo& info, MIMeshInstance* pModelIns);
	void DrawCameraFrustum(MRenderInfo& info, MCamera* pCamera);

	void DrawMeshComponent(MRenderInfo& info, MRenderableMeshComponent* pMeshInstance);

private:

	MIRenderProgram* m_pRenderProgram;
	MForwardRenderShaderParamSet m_FrameParamSet;
};

#endif
