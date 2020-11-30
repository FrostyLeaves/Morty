/**
 * @File         MStandardPostProcessWork
 * 
 * @Created      2020-11-30 17:58:42
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSTANDARDPOSTPROCESSWORK_H_
#define _M_MSTANDARDPOSTPROCESSWORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MIPostProcessWork.h"

#include <array>

class MIMesh;
class MRenderPass;
class MIRenderProgram;
class MIRenderBackTexture;
class MRenderDepthTexture;
class MORTY_CLASS MStandardPostProcessWork : public MIPostProcessWork
{
public:
	M_OBJECT(MStandardPostProcessWork);
	MStandardPostProcessWork();
	virtual ~MStandardPostProcessWork();

public:

	virtual void Initialize(MIRenderProgram* pRenderProgram) override;
	virtual void Release() override;

	virtual MTextureRenderTarget* GetRenderTarget() override;

	virtual void CheckRenderTargetSize(const Vector2& v2Size) override;

	virtual void Render(MPostProcessRenderInfo& info) override;

public:

	void InitializeMesh();
	void ReleaseMesh();

	void InitializeRenderTargets();
	void ReleaseRenderTargets();

	void InitializeRenderPass();
	void ReleaseRenderPass();

protected:

	MIRenderProgram* m_pRenderProgram;
	MTextureRenderTarget* m_pTempRenderTarget;
	MRenderPass* m_pTempRenderPass;
	MIMesh* m_pScreenDrawMesh;

	std::array<MIRenderBackTexture*, M_BUFFER_NUM> m_aBackTexture;
	std::array<MRenderDepthTexture*, M_BUFFER_NUM> m_aDepthTexture;

};
#endif
