#include "MGBufferRenderWork.h"

#include "MVRSTextureRenderWork.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Culling/MInstanceCulling.h"
#include "Mesh/MMeshManager.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"
#include "RenderProgram/MeshRender/MCullingResultRenderable.h"

#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "TaskGraph/MTaskGraph.h"

MORTY_CLASS_IMPLEMENT(MGBufferRenderWork, ISinglePassRenderWork)

const MStringId MGBufferRenderWork::GBufferAlbedoMetallic = MStringId("GBuffer Albedo Metallic Buffer");
const MStringId MGBufferRenderWork::GBufferNormalRoughness = MStringId("GBuffer Normal Roughness Buffer");
const MStringId MGBufferRenderWork::GBufferPositionAmbientOcc = MStringId("GBuffer Position AmbientOcc Buffer");
const MStringId MGBufferRenderWork::GBufferDepthBufferOutput = MStringId("GBuffer Depth Buffer");

void MGBufferRenderWork::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	const Vector2i v2LeftTop = info.f2ViewportLeftTop;
	const Vector2i v2Size = info.f2ViewportSize;


	pCommand->BeginRenderPass(&m_renderPass);
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));


	for (IRenderable* pRenderable : vRenderable)
	{
		pRenderable->Render(pCommand);
	}


	pCommand->EndRenderPass();
}

class MORTY_API MGBufferTextures : public IGBufferAdapter
{
public:
	std::vector<std::shared_ptr<MTexture>> GetBackTextures() const { return vBackTextures; }
	std::shared_ptr<MTexture> GetDepthTexture() const { return pDepthTexture; }

	std::vector<std::shared_ptr<MTexture>> vBackTextures;
	std::shared_ptr<MTexture> pDepthTexture;
};

std::shared_ptr<IGBufferAdapter> MGBufferRenderWork::CreateGBuffer()
{
	auto pGBufferTextures = std::make_shared<MGBufferTextures>();
	pGBufferTextures->vBackTextures = m_renderPass.GetBackTextures();
	pGBufferTextures->pDepthTexture = m_renderPass.GetDepthTexture();

	return pGBufferTextures;
}

void MGBufferRenderWork::Render(const MRenderInfo& info)
{
	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	//Camera frustum culling.

	//Render static mesh.
	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
		GetRenderGraph()->GetFrameProperty(),
	});

	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDeferred));
	indirectMesh.SetInstanceCulling(GetRenderGraph()->GetCameraCullingResult());

	Render(info, {
		&indirectMesh,
		});
}

void MGBufferRenderWork::BindTarget()
{
	AutoBindBarrierTexture();
	SetRenderTarget(AutoBindTargetWithVRS());
}

std::vector<MRenderTaskOutputDesc> MGBufferRenderWork::GetOutputName()
{
    return {
        { GBufferAlbedoMetallic, {true, MColor::Black_T} },
        { GBufferNormalRoughness, {true, MColor::Black_T} },
        { GBufferPositionAmbientOcc, {true, MColor::Black_T} },
        { GBufferDepthBufferOutput, {true, MColor::Black_T} },
    };
}
