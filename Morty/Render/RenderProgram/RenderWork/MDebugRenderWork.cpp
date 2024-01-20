#include "MDebugRenderWork.h"

#include "MForwardRenderWork.h"
#include "MToneMappingRenderWork.h"
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
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"
#include "RenderProgram/MeshRender/MCullingResultRenderable.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"
#include "Resource/MMaterialResource.h"

MORTY_CLASS_IMPLEMENT(MDebugRenderWork, ISinglePassRenderWork)

const MStringId MDebugRenderWork::BackBufferOutput = MStringId("Debug Back Buffer Output");
const MStringId MDebugRenderWork::DepthBufferOutput = MStringId("Debug Depth Buffer Output");

void MDebugRenderWork::Render(const MRenderInfo& info)
{
	//Current viewport.
	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();


	//Render static mesh.
	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({ GetRenderGraph()->GetFrameProperty() });
	indirectMesh.SetMaterialFilter(std::make_shared<MMaterialTypeFilter>(MEMaterialType::ECustom));
	indirectMesh.SetInstanceCulling(GetRenderGraph()->GetCameraCullingResult());

	Render(info, {
		&indirectMesh
		});


}

void MDebugRenderWork::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	//pCommand->AddRenderToTextureBarrier({ CreateOutput()->GetTexture().get() }, METextureBarrierStage::EPixelShaderSample);
	pCommand->AddRenderToTextureBarrier(m_vBarrierTexture, METextureBarrierStage::EPixelShaderSample);

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

void MDebugRenderWork::BindTarget()
{
	AutoBindBarrierTexture();
	SetRenderTarget(AutoBindTarget());
}

std::vector<MStringId> MDebugRenderWork::GetInputName()
{
	return {
		MToneMappingRenderWork::ToneMappingResult,
		MForwardRenderWork::DepthBufferOutput,
	};
}

std::vector<MRenderTaskOutputDesc> MDebugRenderWork::GetOutputName()
{
	return {
		{ BackBufferOutput, {false, MColor::Black_T }},
		{ DepthBufferOutput, {false, MColor::Black_T }}
	};
}