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

class MRenderBackTexture;
class MTextureRenderTarget;
class MORTY_CLASS MForwardTransparentWork : public MObject
{
public:
	M_OBJECT(MForwardTransparentWork);
    MForwardTransparentWork();
    virtual ~MForwardTransparentWork();

public:

	void Initialize(MIRenderProgram* pRenderProgram);
	void Release();

    MIRenderProgram* GetProgram() const { return m_pRenderProgram; }

    void DrawTransparentMesh(MForwardRenderProgram::MRenderInfo& info);

	void RenderToTarget(MForwardRenderProgram::MRenderInfo& info, MRenderPass* pRenderPass, MTextureRenderTarget* pRenderTarget, const uint32_t& unTargetIdx);


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

	void CheckTransparentTextureSize(MForwardRenderProgram::MRenderInfo& info);
    
    void UpdateTextureParams(MForwardRenderProgram::MRenderInfo& info);

    virtual void OnDelete() override;
private:

	MIRenderProgram* m_pRenderProgram;

	MTexture* m_pWhiteTexture;
	MTexture* m_pBlackTexture;

	MTextureRenderTarget* m_pTransparentRenderTarget0;
    MTextureRenderTarget* m_pTransparentRenderTarget1;
    MTextureRenderTarget* m_pTransparentRenderTarget2;

	std::array<MRenderBackTexture*, M_BUFFER_NUM> vBackTexture0;
	std::array<MRenderBackTexture*, M_BUFFER_NUM> vBackTexture1;
	std::array<MRenderBackTexture*, M_BUFFER_NUM> vBackTexture2;
	std::array<MRenderBackTexture*, M_BUFFER_NUM> vBackTexture3;

	MForwardRenderShaderParamSet m_FrameParamSet[3];

	Vector2 m_v2TransparentTextureSize;
	std::array<MRenderBackTexture*, M_BUFFER_NUM> m_vTransparentFrontTexture;
    std::array<MRenderBackTexture*, M_BUFFER_NUM> m_vTransparentBackTexture;
	std::vector<MRenderBackTexture*> m_vRenderTargetTexture;

	MMesh<Vector2> m_TransparentDrawMesh;
    MMaterial* m_pDrawMeshMaterial;


	MRenderPass m_TransWithClearRenderPass;
	MRenderPass m_TransRenderPass;
    MRenderPass m_MeshRenderPass;
};

#endif
