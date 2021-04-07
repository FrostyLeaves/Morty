/**
 * @File         MDeferredGBufferWork
 * 
 * @Created      2021-03-22 16:44:50
 *
 * @Author       Pobrecito
**/

#ifndef _M_MDEFERREDGBUFFERWORK_H_
#define _M_MDEFERREDGBUFFERWORK_H_
#include "MGlobal.h"
#include "MObject.h"

#include "Type/MColor.h"
#include "MRenderPass.h"

#include "MForwardRenderProgram.h"
#include "MForwardRenderShaderParamSet.h"

class MIMesh;
class MMaterial;
class MRenderGraphNode;
class MORTY_API MDeferredGBufferWork : public MObject
{
public:
	M_OBJECT(MDeferredGBufferWork);
    MDeferredGBufferWork();
    virtual ~MDeferredGBufferWork();

public:
	void Initialize(MIRenderProgram* pRenderProgram);
	void Release();

	void InitializeShaderParamSet();
	void ReleaseShaderParamSet();

	void InitializeRenderGraph();
	void ReleaseRenderGraph();

	void InitializeMesh();
	void ReleaseMesh();

	void InitializeMaterial();
	void ReleaseMaterial();


	virtual void OnDelete() override;

	void RenderUpdate(MRenderGraphNode* pGraphNode);

	void Render(MRenderGraphNode* pGraphNode);

	void Lightning(MRenderGraphNode* pGraphNode);


	void DrawNormalMesh(MRenderInfo& info);

	void DrawMeshInstance(MRenderInfo& info, MIMeshInstance* pMeshInstance);

	
private:

	MIRenderProgram* m_pRenderProgram;
	MForwardRenderShaderParamSet m_FrameParamSet;

	MIMesh* m_pScreenDrawMesh;
	MMaterial* m_pLightningMaterial;
};

#endif
