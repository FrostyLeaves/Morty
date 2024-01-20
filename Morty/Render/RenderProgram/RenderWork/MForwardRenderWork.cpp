#include "MForwardRenderWork.h"

#include "MVRSTextureRenderWork.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"
#include "Render/MIDevice.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Model/MSkeleton.h"
#include "Material/MMaterial.h"
#include "Render/MRenderPass.h"
#include "Render/MRenderCommand.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "Culling/MInstanceCulling.h"
#include "Manager/MEnvironmentManager.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"
#include "RenderProgram/MeshRender/MCullingResultRenderable.h"
#include "RenderProgram/MeshRender/MSkyBoxRenderable.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "Resource/MMaterialResource.h"


MORTY_CLASS_IMPLEMENT(MForwardRenderWork, ISinglePassRenderWork)

const MStringId MForwardRenderWork::BackBufferOutput = MStringId("Forward Back Buffer Output");
const MStringId MForwardRenderWork::DepthBufferOutput = MStringId("Forward Depth Buffer Output");



void MForwardRenderWork::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	pCommand->BeginRenderPass(&m_renderPass);

	const Vector2i v2LeftTop = info.f2ViewportLeftTop;
	const Vector2i v2Size = info.f2ViewportSize;
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));


	for (IRenderable* pRenderable : vRenderable)
	{
		pRenderable->Render(pCommand);
	}

	pCommand->EndRenderPass();
}

void MForwardRenderWork::Render(const MRenderInfo& info)
{

	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	//Render static mesh.
	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
		GetRenderGraph()->GetFrameProperty(),
		});
	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::EDefault));
	indirectMesh.SetInstanceCulling(GetRenderGraph()->GetCameraCullingResult());

	const MEnvironmentManager* pEnvironmentManager = info.pScene->GetManager<MEnvironmentManager>();
	const auto pMaterial = pEnvironmentManager->GetMaterial();

	MSkyBoxRenderable skyBox;
	skyBox.SetMesh(pMeshManager->GetSkyBox());
	skyBox.SetMaterial(pMaterial);
	skyBox.SetPropertyBlockAdapter({ GetRenderGraph()->GetFrameProperty() });

	Render(info, {
		&indirectMesh,
		&skyBox,
		});

}

void MForwardRenderWork::BindTarget()
{
	AutoBindBarrierTexture();
	SetRenderTarget(AutoBindTargetWithVRS());
}

std::vector<MStringId> MForwardRenderWork::GetInputName()
{
	return {
		MDeferredLightingRenderWork::DeferredLightingOutput,
		MGBufferRenderWork::GBufferDepthBufferOutput,
		MShadowMapRenderWork::ShadowMapBufferOutput,
	};
}

std::vector<MRenderTaskOutputDesc> MForwardRenderWork::GetOutputName()
{
	return {
		{ BackBufferOutput, {false, MColor::Black_T } },
		{ DepthBufferOutput, {false, MColor::Black_T}}
	};
}
