#include "MGBufferRenderWork.h"

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
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"

MORTY_CLASS_IMPLEMENT(MGBufferRenderWork, ISinglePassRenderWork)

void MGBufferRenderWork::Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	MViewport* pViewport = info.pViewport;
	Vector2 v2LeftTop = pViewport->GetLeftTop();
	Vector2 v2Size = pViewport->GetSize();


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
