/**
 * @File         MForwardRenderWork
 * 
 * @Created      2020-11-29 11:53:33
 *
 * @Author       Pobrecito
**/

#ifndef _M_MFORWARDRENDERWORK_H_
#define _M_MFORWARDRENDERWORK_H_
#include "MGlobal.h"
#include "MObject.h"

#include "Type/MColor.h"
#include "MRenderPass.h"

#include "MForwardRenderProgram.h"
#include "MForwardRenderShaderParamSet.h"

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

	void InitializeRenderPass();
	void ReleaseRenderPass();

	virtual void OnDelete() override;

	void Render(MRenderInfo& info);

	void SetClearColor(const MColor& cClearColor);

	static void UpdateShaderSharedParams(MRenderInfo& info, MForwardRenderShaderParamSet& frameParamSet);

protected:

	void DrawNormalMesh(MRenderInfo& info);

	void DrawModelInstance(MRenderInfo& info);
	void DrawSkyBox(MRenderInfo& info);
	void DrawPainter(MRenderInfo& info);
	void DrawBoundingBox(MRenderInfo& info, MModelInstance* pModelIns);
	void DrawBoundingSphere(MRenderInfo& info, MIMeshInstance* pModelIns);
	void DrawCameraFrustum(MRenderInfo& info, MCamera* pCamera);

	void DrawMeshInstance(MIRenderer* pRenderer, MIMeshInstance* pMeshInstance);

private:

	MIRenderProgram* m_pRenderProgram;
	MForwardRenderShaderParamSet m_FrameParamSet;
	MRenderPass m_ForwardMeshRenderPass;
};

#endif
