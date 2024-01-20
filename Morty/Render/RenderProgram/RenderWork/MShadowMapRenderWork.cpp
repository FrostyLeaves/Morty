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
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"
#include "RenderProgram/MeshRender/MCullingResultRenderable.h"
#include "RenderProgram/RenderGraph/MRenderGraph.h"

#include "Shadow/MShadowMapUtil.h"
#include "Utility/MGlobal.h"


MORTY_CLASS_IMPLEMENT(MShadowMapRenderWork, ISinglePassRenderWork)
const MStringId MShadowMapRenderWork::ShadowMapBufferOutput = MStringId("Shadow Map Buffer Output");

class ShadowMapTexture : public IGetTextureAdapter
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

void MShadowMapRenderWork::Render(const MRenderInfo& info, const std::vector<IRenderable*>& vRenderable)
{
	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;
	if (!pCommand)
		return;

	pCommand->AddRenderToTextureBarrier(m_vBarrierTexture, METextureBarrierStage::EPixelShaderSample);

	const auto& pShadowmap = m_renderPass.GetDepthTexture();

	pCommand->BeginRenderPass(&m_renderPass);

	const Vector2i v2LeftTop = Vector2i(0, 0);
	const Vector2i v2Size = pShadowmap->GetSize2D();
	pCommand->SetViewport(MViewportInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));
	pCommand->SetScissor(MScissorInfo(v2LeftTop.x, v2LeftTop.y, v2Size.x, v2Size.y));

	for (IRenderable* pRenderable : vRenderable)
	{
		pRenderable->Render(pCommand);
	}

	pCommand->EndRenderPass();

	//pCommand->AddRenderToTextureBarrier({ m_renderPass.GetDepthTexture().get() }, METextureBarrierStage::EPixelShaderSample);
}

void MShadowMapRenderWork::Render(const MRenderInfo& info)
{
	const MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();

	MCullingResultRenderable indirectMesh;
	indirectMesh.SetMeshBuffer(pMeshManager->GetMeshBuffer());
	indirectMesh.SetPropertyBlockAdapter({
		GetRenderGraph()->GetFrameProperty(),
	});
	indirectMesh.SetInstanceCulling(GetRenderGraph()->GetShadowCullingResult());

	Render(info, {
		&indirectMesh,
		});
}

std::shared_ptr<IGetTextureAdapter> MShadowMapRenderWork::GetShadowMap() const
{
	auto pShadowMap = std::make_shared<ShadowMapTexture>();
	pShadowMap->pTexture = m_renderPass.GetDepthTexture();

	return pShadowMap;
}

class MORTY_API MShadowPropertyDecorator : public IShaderPropertyUpdateDecorator
{
public:

	explicit MShadowPropertyDecorator(MShadowMapRenderWork* pOwner) : m_pOwner(pOwner) {}

	void BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock) override
	{
		MORTY_ASSERT(m_pShadowTextureParam = pShaderPropertyBlock->FindTextureParam(MShaderPropertyName::TEXTURE_SHADOW_MAP));
	}

	void Update(const MRenderInfo& info) override
	{
		MORTY_UNUSED(info);

		const auto pTexture = m_pOwner->GetShadowMap()->GetTexture();
		if (m_pShadowTextureParam && m_pShadowTextureParam->GetTexture() != pTexture)
		{
			m_pShadowTextureParam->SetTexture(pTexture);
		}
	}

	std::shared_ptr<MShaderTextureParam> m_pShadowTextureParam = nullptr;

	MShadowMapRenderWork* m_pOwner = nullptr;
};

std::shared_ptr<IShaderPropertyUpdateDecorator> MShadowMapRenderWork::GetFramePropertyDecorator()
{
	return std::make_shared<MShadowPropertyDecorator>(this);
}

void MShadowMapRenderWork::OnCreated()
{
	Super::OnCreated();
}

void MShadowMapRenderWork::BindTarget()
{
	AutoBindBarrierTexture();
	SetRenderTarget(AutoBindTarget());
}

std::vector<MRenderTaskOutputDesc> MShadowMapRenderWork::GetOutputName()
{
    return {
        { ShadowMapBufferOutput, { true, MColor::White } },
    };
}
