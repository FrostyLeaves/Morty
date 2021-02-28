/**
 * @File         MForwardTransparentRenderProgram
 * 
 * @Created      2020-08-25 11:25:01
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFORWARDTRANSPARENTWORK_H_
#define _M_MFORWARDTRANSPARENTWORK_H_
#include "MGlobal.h"
#include "MObject.h"
#include "MForwardRenderProgram.h"

#include <array>

class MIRenderTexture;
class MRenderTexture;
class MRenderTexture;
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

    void Render(MRenderGraphNode* pGraphNode);

	void RenderDepthPeel(MRenderGraphNode* pGraphNode);


protected:

    void InitializeMesh();
    void ReleaseMesh();

    void InitializeMaterial();
    void ReleaseMaterial();

    void InitializeTexture();
    void ReleaseTexture();

	void InitializeRenderGraph();

	void BindTextureParam();

    void SetupSubPass(MRenderPass& renderpass);

    virtual void OnDelete() override;
private:

	MIRenderProgram* m_pRenderProgram;

	MTexture* m_pWhiteTexture;
	MTexture* m_pBlackTexture;

	MForwardRenderTransparentShaderParamSet m_aFrameParamSet[2];

	Vector2 m_v2TransparentTextureSize;
	std::vector<MIRenderTexture*> m_vRenderTargetTexture;

	MMesh<Vector2> m_TransparentDrawMesh;
    MMaterial* m_pDrawMeshMaterial;
    MMaterial* m_pDrawFillMaterial;
};

#endif
