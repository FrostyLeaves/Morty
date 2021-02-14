/**
 * @File         MForwardShadowMapWork
 * 
 * @Created      2020-08-25 11:25:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFORWARD_SHADOWMAP_WORK_H_
#define _M_MFORWARD_SHADOWMAP_WORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MForwardRenderProgram.h"

#include <array>

class MIRenderTexture;
struct MShadowRenderGroup
{
	MShadowRenderGroup() :pSkeletonInstance(nullptr) {}
	MSkeletonInstance* pSkeletonInstance;
	std::vector<MIMeshInstance*> vMeshInstances;
};

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

    void UpdateRenderInfo(MRenderInfo& info, std::vector<MShadowRenderGroup>& vShadowMeshGroup);

    void RenderMesh(MRenderInfo& info, std::vector<MShadowRenderGroup>& vShadowMeshGroup);

    void Render(MRenderGraphNode* pGraphNode);

protected:

	void InitializeRenderTargets();
    void ReleaseRenderTargets();

	void InitializeShaderParamSet();
	void ReleaseShaderParamSet();

    void InitializeMaterial();
    void ReleaseMaterial();

    void InitializeRenderGraph();

    virtual void OnDelete() override;
private:

    MIRenderProgram* m_pRenderProgram;

    MShaderParamSet m_FrameParamSet;
    MShaderConstantParam* m_pWorldMatrixParam;

	MMaterial* m_pStaticMaterial;
	MMaterial* m_pAnimMaterial;
};

#endif
