/**
 * @File         MGaussianBlurWork
 * 
 * @Created      2020-11-30 17:43:05
 *
 * @Author       Pobrecito
**/

#ifndef _M_MGAUSSIANBLURWORK_H_
#define _M_MGAUSSIANBLURWORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MStandardPostProcessWork.h"

class MMaterial;
class MShaderParamSet;
class MORTY_CLASS MGaussianBlurWork : public MIPostProcessWork
{
public:
	M_OBJECT(MGaussianBlurWork);
    MGaussianBlurWork();
    virtual ~MGaussianBlurWork();

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


    void InitializeMaterial();
    void ReleaseMaterial();

protected:

	MIRenderProgram* m_pRenderProgram;
	MTextureRenderTarget* m_aTempRenderTarget[2];
	std::array<MIRenderBackTexture*, M_BUFFER_NUM> m_aBackTexture[2];

	MRenderPass* m_pTempRenderPass;
	MIMesh* m_pScreenDrawMesh;


    MMaterial* m_aMaterial[3];
};

#endif
