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
#include "Render/MVertex.h"

#include "Utility/MBounds.h"
#include "Mesh/MMeshManager.h"
#include "System/MObjectSystem.h"

#include "Material/MComputeDispatcher.h"

MORTY_INTERFACE_IMPLEMENT(MVRSTextureRenderWork, MTypeClass)

void MVRSTextureRenderWork::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	m_pVRSTexture = MTexture::CreateShadingRate();

	auto pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	m_n2TexelSize = pRenderSystem->GetDevice()->GetShadingRateTextureTexelSize();

	m_pVRSGenerator = pObjectSystem->CreateObject<MComputeDispatcher>();
	m_pVRSGenerator->GetShaderProgram()->GetShaderMacro().AddUnionMacro(MStringId("MORTY_SHADING_RATE_TEXEL_SIZE"), MStringUtil::ToString(m_n2TexelSize.x));
	m_pVRSGenerator->LoadComputeShader("Shader/VRS/vrs_image_from_edge_detection.mcs");

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pVRSGenerator->GetShaderPropertyBlock(0);

	params->SetValue(MShaderPropertyName::VRS_TEXEL_SIZE_NAME, Vector2(m_n2TexelSize.x, m_n2TexelSize.y));
	params->SetValue(MShaderPropertyName::VRS_EDGE_THRESHOLD_NAME, Vector2(1.0f, 10.0f));

	params->SetTexture(MShaderPropertyName::VRS_OUTPUT_VRS_TEXTURE_NAME, m_pVRSTexture);

	m_pVRSTexture->GenerateBuffer(pRenderSystem->GetDevice());
}

void MVRSTextureRenderWork::Release(MEngine* pEngine)
{
	m_pVRSGenerator->DeleteLater();
	m_pVRSGenerator = nullptr;

	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();
	m_pVRSTexture->DestroyBuffer(pRenderSystem->GetDevice());
}

std::shared_ptr<MTexture> MVRSTextureRenderWork::GetVRSTexture() const
{
	return m_pVRSTexture;
}

void MVRSTextureRenderWork::Resize(Vector2i size)
{
	MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
	Vector2i n2TexelSize = pRenderSystem->GetDevice()->GetShadingRateTextureTexelSize();
	Vector2i n2ShadingRateSize = {};
	n2ShadingRateSize.x = size.x / n2TexelSize.x + ((size.x % n2TexelSize.x) != 0);
	n2ShadingRateSize.y = size.y / n2TexelSize.y + ((size.y % n2TexelSize.y) != 0);


	if (m_pVRSTexture->GetSize2D() != n2ShadingRateSize)
	{
		m_pVRSTexture->SetSize(n2ShadingRateSize);
		m_pVRSTexture->DestroyBuffer(pRenderSystem->GetDevice());
		m_pVRSTexture->GenerateBuffer(pRenderSystem->GetDevice());
	}

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pVRSGenerator->GetShaderPropertyBlock(0);
	params->SetValue(MShaderPropertyName::VRS_EDGE_TEXTURE_SIZE_NAME, Vector2(size.x, size.y));
}

void MVRSTextureRenderWork::Render(MRenderInfo& info, const std::shared_ptr<IGetTextureAdapter>& pEdgeTextureAdapter)
{
	if (!pEdgeTextureAdapter)
	{
		return;
	}

	const auto pEdgeTexture = pEdgeTextureAdapter->GetTexture();
	if (!pEdgeTexture)
	{
		return;
	}

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pVRSGenerator->GetShaderPropertyBlock(0);

	const Vector2i n2ThreadNum = {
		pEdgeTexture->GetSize2D().x / m_n2TexelSize.x + (pEdgeTexture->GetSize2D().x % m_n2TexelSize.x != 0),
		pEdgeTexture->GetSize2D().y / m_n2TexelSize.y + (pEdgeTexture->GetSize2D().y % m_n2TexelSize.y != 0)
	};

	params->SetTexture(MShaderPropertyName::VRS_EDGE_TEXTURE_NAME, pEdgeTexture);

	info.pPrimaryRenderCommand->AddRenderToTextureBarrier({ pEdgeTexture.get() }, METextureBarrierStage::EComputeShaderRead);

	info.pPrimaryRenderCommand->AddRenderToTextureBarrier({ m_pVRSTexture.get() }, METextureBarrierStage::EComputeShaderWrite);

	info.pPrimaryRenderCommand->DispatchComputeJob(m_pVRSGenerator, n2ThreadNum.x, n2ThreadNum.y, 1);
}
