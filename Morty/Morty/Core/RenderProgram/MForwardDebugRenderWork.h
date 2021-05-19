/**
 * @File         MForwardDebugRenderWork
 * 
 * @Created      2021-05-17 15:21:55
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFORWARD_DEBUG_RENDER_WORK_H_
#define _M_MFORWARD_DEBUG_RENDER_WORK_H_
#include "MGlobal.h"
#include "MObject.h"

#include "Type/MColor.h"
#include "MRenderPass.h"

#include "MMesh.h"
#include "MPainter.h"

#include "MForwardRenderProgram.h"
#include "MForwardRenderShaderParamSet.h"

class MRenderGraphNode;
class MModelComponent;
class MRenderableMeshComponent;
class MORTY_API MForwardDebugRenderWork : public MObject
{
public:
	M_OBJECT(MForwardDebugRenderWork);
	MForwardDebugRenderWork();
    virtual ~MForwardDebugRenderWork();

public:
	void Initialize(MIRenderProgram* pRenderProgram);
	void Release();

	void InitializeShaderParamSet();
	void ReleaseShaderParamSet();

	void InitializeRenderGraph();
	void ReleaseRenderGraph();

	void InitializeMaterial();
	void ReleaseMaterial();

	virtual void OnDelete() override;

	void Render(MRenderGraphNode* pGraphNode);

protected:

	void DrawModelBoundingBox(MRenderInfo& info);
	void DrawPainter(MRenderInfo& info);
	void FillBoundingBoxMesh(MRenderInfo& info, MModelComponent* pModelComponent);
	void DrawBoundingSphere(MRenderInfo& info, MIMeshInstance* pModelIns);
	void DrawCameraFrustum(MRenderInfo& info, MCamera* pCamera);

private:

	MIRenderProgram* m_pRenderProgram;

	MMaterial* m_pBoundingDrawMaterial;
	MMaterial* m_pTransformCoordDrawMaterial;

	MMesh<MPainterVertex> m_BoundingDrawMesh;
	MMesh<MPainterVertex> m_TransformCoordDrawMesh;
};

#endif
