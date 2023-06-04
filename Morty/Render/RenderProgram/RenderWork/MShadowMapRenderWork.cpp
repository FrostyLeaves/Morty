#include "MShadowMapRenderWork.h"

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

#include "Shadow/MShadowMapUtil.h"


MORTY_CLASS_IMPLEMENT(MShadowMapRenderWork, ISinglePassRenderWork)

class ShadowMapTexture : public ITextureInputAdapter
{
public:
	virtual std::shared_ptr<MTexture> GetTexture() { return pTexture; }

	std::shared_ptr<MTexture> pTexture;
};

void MShadowMapRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);
	m_renderPass.SetViewportNum(MRenderGlobal::CASCADED_SHADOW_MAP_NUM);
}

void MShadowMapRenderWork::Render(MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	MIDevice* pRenderDevice = pRenderSystem->GetDevice();

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	if (!pCommand)
		return;

	const auto& pShadowmap = m_renderPass.GetDepthTexture();

	pCommand->BeginRenderPass(&m_renderPass);

	Vector2 v2LeftTop = Vector2(0.0f, 0.0f);
	Vector2 v2Size = pShadowmap->GetSize();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));

	for (IRenderable* pRenderable : vRenderable)
	{
		pRenderable->Render(pCommand);
	}

	pCommand->EndRenderPass();
}

void MShadowMapRenderWork::Resize(Vector2 size)
{
    //Shadow map can`t resize.
}

std::shared_ptr<ITextureInputAdapter> MShadowMapRenderWork::GetShadowMap() const
{
	auto pShadowMap = std::make_shared<ShadowMapTexture>();
	pShadowMap->pTexture = m_renderPass.GetDepthTexture();

	return pShadowMap;
}
