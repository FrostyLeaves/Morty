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

class MTextureRenderTarget;
struct MShadowRenderGroup
{
	MShadowRenderGroup() :pSkeletonInstance(nullptr) {}
	MSkeletonInstance* pSkeletonInstance;
	std::vector<MIMeshInstance*> vMeshInstances;
};

class MRenderDepthTexture;
class MShadowTextureRenderTarget;
class MORTY_API MForwardShadowMapWork : public MObject
{
public:
	M_OBJECT(MForwardShadowMapWork);
    MForwardShadowMapWork();
    virtual ~MForwardShadowMapWork();

public:

    void Initialize(MIRenderProgram* pRenderProgram);
    void Release();

    MIRenderProgram* GetProgram() const { return m_pRenderProgram; }

    void Render(MRenderInfo& info);

    void UpdateRenderInfo(MRenderInfo& info, std::vector<MShadowRenderGroup>& vShadowMeshGroup);

    void RenderMesh(MRenderInfo& info, std::vector<MShadowRenderGroup>& vShadowMeshGroup);

    MRenderDepthTexture* GetShadowMap(const uint32_t& unIdx) { return m_vShadowDepthTexture[unIdx]; }

protected:

	void InitializeRenderTargets();
    void ReleaseRenderTargets();

	void InitializeShaderParamSet();
	void ReleaseShaderParamSet();

    void InitializeMaterial();
    void ReleaseMaterial();

    void InitializeRenderPass();
    void ReleaseRenderPass();

    virtual void OnDelete() override;
private:

    MIRenderProgram* m_pRenderProgram;

	MTextureRenderTarget* m_pShadowDepthMapRenderTarget;

	std::array<MRenderDepthTexture*, M_BUFFER_NUM> m_vShadowDepthTexture;

    MShaderParamSet m_FrameParamSet;
    MShaderConstantParam* m_pWorldMatrixParam;

	MMaterial* m_pStaticMaterial;
	MMaterial* m_pAnimMaterial;

    MRenderPass m_ShadowRenderPass;
};

#endif
