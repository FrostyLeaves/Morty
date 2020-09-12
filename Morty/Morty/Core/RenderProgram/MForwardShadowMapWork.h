/**
 * @File         MForwardShadowMapWork
 * 
 * @Created      2020-08-25 11:25:01
 *
 * @Author       Pobrecito
**/

#ifndef _M_MFORWARD_SHADOWMAP_WORK_H_
#define _M_MFORWARD_SHADOWMAP_WORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MForwardRenderProgram.h"

#include <array>

class MRenderDepthTexture;
class MShadowTextureRenderTarget;
class MORTY_CLASS MForwardShadowMapWork : public MObject
{
public:
	M_OBJECT(MForwardShadowMapWork);
    MForwardShadowMapWork();
    virtual ~MForwardShadowMapWork();

public:

    void SetProgram(MIRenderProgram* pRenderProgram) { m_pRenderProgram = pRenderProgram; }
    MIRenderProgram* GetProgram() const { return m_pRenderProgram; }

    void DrawShadowMap(MForwardRenderProgram::MRenderInfo& info);

    void UpdateRenderInfo(MForwardRenderProgram::MRenderInfo& info);

    void RenderToShadowMap(MForwardRenderProgram::MRenderInfo& info);

    virtual void OnCreated() override;
    virtual void OnDelete() override;

    MRenderDepthTexture* GetShadowMap(const uint32_t& unIdx) { return m_vShadowDepthTexture[unIdx]; }

protected:

	void InitializeRenderTargets();
    void ReleaseRenderTargets();

	void InitializeShaderParamSet();
	void ReleaseShaderParamSet();

    void InitializeMaterial();
    void ReleaseMaterial();
private:

    MIRenderProgram* m_pRenderProgram;

	MShadowTextureRenderTarget* m_pShadowDepthMapRenderTarget;

	std::array<MRenderDepthTexture*, M_BUFFER_NUM> m_vShadowDepthTexture;

    MShaderParamSet m_FrameParamSet;
    MShaderConstantParam* m_pWorldMatrixParam;

	MMaterial* m_pStaticMaterial;
	MMaterial* m_pAnimMaterial;
};

#endif
