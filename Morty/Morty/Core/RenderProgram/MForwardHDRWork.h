/**
 * @File         MForwardHDRWork
 * 
 * @Created      2020-11-29 11:05:51
 *
 * @Author       Pobrecito
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
class MGaussianBlurWork;
class MIRenderBackTexture;
class MRenderDepthTexture;
class MORTY_CLASS MForwardHDRWork : public MIPostProcessWork
{
public:
	M_OBJECT(MForwardHDRWork);
    MForwardHDRWork();
    virtual ~MForwardHDRWork();

public:

    virtual void Render(MPostProcessRenderInfo& info) override;

	virtual MTextureRenderTarget* GetRenderTarget();

	virtual void Initialize(MIRenderProgram* pRenderProgram) override;
	virtual void Release() override;

	void InitializeMaterial();
	void ReleaseMaterial();

	void InitializeMesh();
	void ReleaseMesh();

	void InitializeRenderTargets();
	void ReleaseRenderTargets();

	void InitializeRenderPass();
	void ReleaseRenderPass();

	virtual void CheckRenderTargetSize(const Vector2& v2Size) override;

protected:

	void RenderBloom(MPostProcessRenderInfo& info);
	void RenderCombine(MPostProcessRenderInfo& info);

protected:

	MIRenderProgram* m_pRenderProgram;
	MTextureRenderTarget* m_pTempRenderTarget;
	MRenderPass* m_pTempRenderPass;
	MIMesh* m_pScreenDrawMesh;
	MMaterial* m_pHDRMaterial;

	std::array<MIRenderBackTexture*, M_BUFFER_NUM> m_aBackTexture;
	std::array<MIRenderBackTexture*, M_BUFFER_NUM> m_aHighLightTexture;

	std::array<MTexture*, M_BUFFER_NUM> m_aLumTexture;
private:

	MGaussianBlurWork* m_pGaussianBlurWork;
	MCombineWork* m_pCombineWork;

	float m_fAverageLum;
	float m_fAdaptLum;
	float m_fAdjustLum;
};

#endif
