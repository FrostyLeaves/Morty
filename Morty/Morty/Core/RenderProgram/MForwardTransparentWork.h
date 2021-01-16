/**
 * @File         MForwardTransparentRenderProgram
 * 
 * @Created      2020-08-25 11:25:01
 *
 * @Author       Pobrecito
**/

#ifndef _M_MFORWARDTRANSPARENTWORK_H_
#define _M_MFORWARDTRANSPARENTWORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MForwardRenderProgram.h"

#include <array>

class MIRenderBackTexture;
class MRenderBackTexture;
class MRenderSubpassTexture;
class MTextureRenderTarget;
class MORTY_API MForwardTransparentWork : public MObject
{
public:
	M_OBJECT(MForwardTransparentWork);
    MForwardTransparentWork();
    virtual ~MForwardTransparentWork();

public:

	void Initialize(MIRenderProgram* pRenderProgram);
	void Release();

    MIRenderProgram* GetProgram() const { return m_pRenderProgram; }

    void Render(MRenderInfo& info);

	void RenderDepthPeel(MRenderInfo& info, MRenderPass* pRenderPass, MTextureRenderTarget* pRenderTarget, const uint32_t& unTargetIdx);


protected:

    void InitializeMesh();
    void ReleaseMesh();

    void InitializeMaterial();
    void ReleaseMaterial();

    void InitializeTexture();
    void ReleaseTexture();

    void InitializeRenderTargets();
    void ReleaseRenderTargets();

    void InitializeRenderPass();
    void ReleaseRenderPass();

	void CheckTransparentTextureSize(MRenderInfo& info);

	void UpdateShaderSharedParams(MRenderInfo& info);

    void SetupSubPass(MRenderPass& renderpass);

    virtual void OnDelete() override;
private:

	MIRenderProgram* m_pRenderProgram;

	MTexture* m_pWhiteTexture;
	MTexture* m_pBlackTexture;

	MTextureRenderTarget* m_pTransparentRenderTarget;

	std::array<MIRenderBackTexture*, M_BUFFER_NUM> vBackTexture;
	std::array<MIRenderBackTexture*, M_BUFFER_NUM> vFrontTexture;
	std::array<MIRenderBackTexture*, M_BUFFER_NUM> vBackDepthTexture[2];
	std::array<MIRenderBackTexture*, M_BUFFER_NUM> vFrontDepthTexture[2];

	MForwardRenderTransparentShaderParamSet m_aFrameParamSet[2];

	Vector2 m_v2TransparentTextureSize;
	std::vector<MIRenderBackTexture*> m_vRenderTargetTexture;

	MMesh<Vector2> m_TransparentDrawMesh;
    MMaterial* m_pDrawMeshMaterial;
    MMaterial* m_pDrawFillMaterial;


	MRenderPass m_TransWithClearRenderPass;
    MRenderPass m_MeshRenderPass;
};

#endif
