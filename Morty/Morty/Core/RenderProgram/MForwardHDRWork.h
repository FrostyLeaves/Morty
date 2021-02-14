/**
 * @File         MForwardHDRWork
 * 
 * @Created      2020-11-29 11:05:51
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFORWARDHDRWORK_H_
#define _M_MFORWARDHDRWORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MIPostProcessWork.h"

#include <array>

class MIMesh;
class MTexture;
class MMaterial;
class MRenderPass;
class MCombineWork;
class MIRenderProgram;
class MIRenderTexture;
class MRenderGraphNode;
class MGaussianBlurWork;
class MORTY_API MForwardHDRWork : public MIPostProcessWork
{
public:
	M_OBJECT(MForwardHDRWork);
    MForwardHDRWork();
    virtual ~MForwardHDRWork();

public:

	virtual void Render(MRenderGraphNode* pGraphNode);

	virtual void Initialize(MIRenderProgram* pRenderProgram) override;
	virtual void Release() override;

	void InitializeMaterial();
	void ReleaseMaterial();

	void InitializeMesh();
	void ReleaseMesh();

	void InitializeRenderGraph();
	void ReleaseRenderGraph();


	void InitializeCopyTarget();
	void ReleaseCopyTarget();


protected:

	MIRenderProgram* m_pRenderProgram;
	MIMesh* m_pScreenDrawMesh;
	MMaterial* m_pHDRMaterial;

	std::array<MTexture*, M_BUFFER_NUM> m_aLumTexture;
private:

	MGaussianBlurWork* m_pGaussianBlurWork;
	MCombineWork* m_pCombineWork;

	float m_fAverageLum;
	float m_fAdaptLum;
	float m_fAdjustLum;
};

#endif
