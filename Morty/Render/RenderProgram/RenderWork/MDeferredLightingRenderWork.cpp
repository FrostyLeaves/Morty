#include "MDeferredLightingRenderWork.h"

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
#include "Render/MMaterialName.h"
#include "Resource/MMaterialResource.h"


MORTY_CLASS_IMPLEMENT(MDeferredLightingRenderWork, ISinglePassRenderWork)

void MDeferredLightingRenderWork::Render(MRenderInfo& info)
{
	if (!m_pFramePropertyAdapter)
	{
		MORTY_ASSERT(m_pFramePropertyAdapter);
		return;
	}

	if (!m_pGBufferAdapter)
	{
		MORTY_ASSERT(m_pGBufferAdapter);
		return;
	}

	if (!m_pShadowMapAdapter)
	{
		MORTY_ASSERT(m_pShadowMapAdapter);
		return;
	}

	MMeshManager* pMeshManager = GetEngine()->FindGlobalObject<MMeshManager>();
	if (!pMeshManager)
	{
		MORTY_ASSERT(pMeshManager);
		return;
	}

	MIRenderCommand* pCommand = info.pPrimaryRenderCommand;

	std::vector<MTexture*> vTextures;
	for (auto pTexture : m_pGBufferAdapter->GetBackTextures())
	{
		vTextures.push_back(pTexture.get());
	}
	vTextures.push_back(m_pGBufferAdapter->GetDepthTexture().get());

	pCommand->AddRenderToTextureBarrier(vTextures, METextureBarrierStage::EPixelShaderSample);

	pCommand->BeginRenderPass(&m_renderPass);

	//pCommand->SetShadingRate({ 1, 1 }, { MEShadingRateCombinerOp::Keep, MEShadingRateCombinerOp::Replace });

	const Vector2i n2Size = m_renderPass.GetFrameBufferSize();

	pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, n2Size.x, n2Size.y));
	pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, n2Size.x, n2Size.y));


	if (pCommand->SetUseMaterial(m_pLightningMaterial))
	{
		auto pPropertyBlock = m_pFramePropertyAdapter->GetPropertyBlock();
		pCommand->SetShaderPropertyBlock(pPropertyBlock);
		
		pCommand->DrawMesh(pMeshManager->GetScreenRect());
	}

	pCommand->EndRenderPass();
}

void MDeferredLightingRenderWork::Initialize(MEngine* pEngine)
{
	Super::Initialize(pEngine);

	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	const auto pTemplate = pResourceSystem->LoadResource(MMaterialName::DEFERRED_LIGHTING);
	m_pLightningMaterial = MMaterial::CreateMaterial(pTemplate);

}

void MDeferredLightingRenderWork::Release(MEngine* pEngine)
{
	m_pLightningMaterial = nullptr;

	auto pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	if (auto pShadingRateTexture = m_renderPass.GetShadingRateTexture())
	{
		pShadingRateTexture->DestroyBuffer(pRenderSystem->GetDevice());
	}

	Super::Release(pEngine);
}

void MDeferredLightingRenderWork::Resize(Vector2i size)
{
	/*
	auto pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

	if (auto pShadingRateTexture = m_renderPass.GetShadingRateTexture())
	{
		Vector2i n2TexelSize = pRenderSystem->GetDevice()->GetShadingRateTextureTexelSize();
		Vector2i n2ShadingRateSize = {};
		n2ShadingRateSize.x = size.x / n2TexelSize.x + ((size.x % n2TexelSize.x) != 0);
		n2ShadingRateSize.y = size.y / n2TexelSize.y + ((size.y % n2TexelSize.y) != 0);

		if (pShadingRateTexture->GetSize2D().x != n2ShadingRateSize.x || pShadingRateTexture->GetSize2D().y != n2ShadingRateSize.y)
		{
			std::vector<MByte> data(n2ShadingRateSize.x * n2ShadingRateSize.y * sizeof(MByte));

			for (int h = 0; h < n2ShadingRateSize.y; ++h)
			{
				for (int w = 0; w < n2ShadingRateSize.x; ++w)
				{
					MByte pixel = 0;

					if (w < n2ShadingRateSize.x / 2)
					{
						pixel = MShadingRateType::Rate_1x1;
					}
					else
					{
						pixel = MShadingRateType::Rate_4X4;
					}

					data[h * n2ShadingRateSize.x + w] = pixel;
				}
			}


			pShadingRateTexture->SetSize(n2ShadingRateSize);
			pShadingRateTexture->DestroyBuffer(pRenderSystem->GetDevice());
			pShadingRateTexture->GenerateBuffer(pRenderSystem->GetDevice(), data.data());
		}
	}
	*/

	ISinglePassRenderWork::Resize(size);

}

void MDeferredLightingRenderWork::SetGBuffer(const std::shared_ptr<IGBufferAdapter>& pAdapter)
{
	MORTY_ASSERT(m_pGBufferAdapter = pAdapter);

	if (std::shared_ptr<MShaderPropertyBlock> pParams = m_pLightningMaterial->GetMaterialPropertyBlock())
	{
		const auto& vTextures = pAdapter->GetBackTextures();
		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_ALBEDO_METALLIC, vTextures[0]);
		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_NORMAL_ROUGHNESS, vTextures[1]);
		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_POSITION_AMBIENTOCC, vTextures[2]);
		pParams->SetTexture(MShaderPropertyName::GBUFFER_TEXTURE_DEPTH_MAP, pAdapter->GetDepthTexture());
	}
}

void MDeferredLightingRenderWork::SetShadowMap(const std::shared_ptr<IGetTextureAdapter>& pAdapter)
{
	m_pShadowMapAdapter = pAdapter;
}

void MDeferredLightingRenderWork::SetFrameProperty(const std::shared_ptr<IPropertyBlockAdapter>& pAdapter)
{
	m_pFramePropertyAdapter = pAdapter;
}
