/**
 * @File         MForwardPostProcessProgram
 * 
 * @Created      2020-11-29 15:32:27
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFORWARDPOSTPROCESSPROGRAM_H_
#define _M_MFORWARDPOSTPROCESSPROGRAM_H_
#include "MGlobal.h"
#include "MEngine.h"
#include "MForwardRenderProgram.h"

class MMaterial;
class MRenderPass;
class MForwardHDRWork;
class MIPostProcessWork;
class MIRenderTexture;
class MORTY_API MForwardPostProcessProgram : public MForwardRenderProgram
{
public:
	M_OBJECT(MForwardPostProcessProgram);
    MForwardPostProcessProgram();
    virtual ~MForwardPostProcessProgram();

public:
	virtual void Render(MIRenderer* pRenderer, MViewport* pViewport) override;

	template<typename CLASS_TYPE>
	CLASS_TYPE* AppendPostProcess();


	void SetHighDynamicRangeEnable(const bool& bEnable);
	bool GetHighDynamicRangeEnable() { return m_bHDR_Enable; }

protected:


	void RenderPostProcess(const MRenderInfo& info);
	void RenderScreenMesh(const MRenderInfo& info);

	void CheckRenderTargetSize(const Vector2& v2Size);

	virtual void Initialize() override;
	virtual void Release() override;

	void InitializeMesh();
	void ReleaseMesh();

	void InitializeMaterial();
	void ReleaseMaterial();

	void InitializeRenderPass();
	void ReleaseRenderPass();

	void InitializeRenderTarget();
	void ReleaseRenderTarget();

protected:

	//HDR first
	bool m_bHDR_Enable;
	MIPostProcessWork* m_pHDRPostProcessWork;
	std::vector<MIPostProcessWork*> m_vPostProcessWork;

private:

	std::array<MIRenderTexture*, M_BUFFER_NUM> m_aBackTexture;
	std::array<MIRenderTexture*, M_BUFFER_NUM> m_aDepthTexture;

	MTextureRenderTarget* m_pTempRenderTarget;
	MRenderPass* m_pScreenDrawRenderPass;
	MIMesh* m_pScreenDrawMesh;
	MMaterial* m_pScreenDrawMaterial;
};

template<typename CLASS_TYPE>
CLASS_TYPE* MForwardPostProcessProgram::AppendPostProcess()
{
	CLASS_TYPE* pPostProcess = GetEngine()->GetObjectManager()->CreateObject<CLASS_TYPE>();
	pPostProcess->Initialize(this);

	m_vPostProcessWork.push_back(pPostProcess);

	return pPostProcess;
}

#endif
