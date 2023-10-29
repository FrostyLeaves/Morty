#include "MForwardRenderWork.h"

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
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"
#include "Resource/MMaterialResource.h"


MORTY_CLASS_IMPLEMENT(MForwardRenderWork, ISinglePassRenderWork)

void MForwardRenderWork::Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
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
