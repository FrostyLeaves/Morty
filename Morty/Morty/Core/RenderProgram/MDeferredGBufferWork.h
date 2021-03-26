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

	virtual void OnDelete() override;

	void Render(MRenderGraphNode* pGraphNode);

	static void UpdateShaderSharedParams(MRenderInfo& info, MForwardRenderShaderParamSet& frameParamSet);

private:

	MIRenderProgram* m_pRenderProgram;
};

#endif
