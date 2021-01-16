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
class MORTY_API MGaussianBlurWork : public MIPostProcessWork
{
public:
	M_OBJECT(MGaussianBlurWork);
    MGaussianBlurWork();
    virtual ~MGaussianBlurWork();

public:

	void SetBlurRadius(const float& fRadius) { m_fBlurRadius = fRadius; }
	float GetBlurRadius() const { return m_fBlurRadius; }

	void SetIteration(const uint32_t& unIteration) { m_unIteration = unIteration; }
	uint32_t GetIteration() const { return m_unIteration; }

public:

	virtual void Initialize(MIRenderProgram* pRenderProgram) override;
	virtual void Release() override;

	virtual MTextureRenderTarget* GetRenderTarget() override;

	virtual void CheckRenderTargetSize(const Vector2& v2Size) override;

	virtual void Render(MPostProcessRenderInfo& info) override;

protected:

	void UpdateShaderSharedParams(MPostProcessRenderInfo& info);

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

private:
	float m_fBlurRadius;
	uint32_t m_unIteration;
};

#endif
