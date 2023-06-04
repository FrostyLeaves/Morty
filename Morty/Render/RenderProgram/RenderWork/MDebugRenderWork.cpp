#include "MDebugRenderWork.h"

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
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"
#include "Resource/MMaterialResource.h"

MORTY_CLASS_IMPLEMENT(MDebugRenderWork, ISinglePassRenderWork)

void MDebugRenderWork::Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MViewport* pViewport = info.pViewport;
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	pCommand->AddRenderToTextureBarrier({ CreateOutput()->GetTexture().get() });

	pCommand->BeginRenderPass(&m_renderPass);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	Vector2 v2Size = pViewport->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, v2Size.x, v2Size.y));


	for (IRenderable* pRenderable : vRenderable)
	{
		pRenderable->Render(pCommand);
	}


	pCommand->EndRenderPass();
}
