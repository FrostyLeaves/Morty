#include "MVRSTextureRenderWork.h"

#include "MEdgeDetectionRenderWork.h"
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

MORTY_CLASS_IMPLEMENT(MVRSTextureRenderWork, MRenderTaskNode)

const MStringId MVRSTextureRenderWork::VRS_TEXTURE = MStringId("VRS Screen Texture");

void MVRSTextureRenderWork::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	auto pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

	m_n2TexelSize = pRenderSystem->GetDevice()->GetShadingRateTextureTexelSize();

	m_pVRSGenerator = pObjectSystem->CreateObject<MComputeDispatcher>();
	m_pVRSGenerator->GetShaderProgram()->GetShaderMacro().AddUnionMacro(MStringId("MORTY_SHADING_RATE_TEXEL_SIZE"), MStringUtil::ToString(m_n2TexelSize.x));
	m_pVRSGenerator->LoadComputeShader("Shader/VRS/vrs_image_from_edge_detection.mcs");

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pVRSGenerator->GetShaderPropertyBlock(0);

	params->SetValue(MShaderPropertyName::VRS_TEXEL_SIZE_NAME, Vector2(m_n2TexelSize.x, m_n2TexelSize.y));
	params->SetValue(MShaderPropertyName::VRS_EDGE_THRESHOLD_NAME, Vector2(1.0f, 10.0f));

}

void MVRSTextureRenderWork::Release()
{
	m_pVRSGenerator->DeleteLater();
	m_pVRSGenerator = nullptr;
}

void MVRSTextureRenderWork::Resize(Vector2i size)
{
	MORTY_UNUSED(size);

	const auto pVRSTexture = GetRenderOutput(0)->GetTexture();

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pVRSGenerator->GetShaderPropertyBlock(0);
	params->SetValue(MShaderPropertyName::VRS_EDGE_TEXTURE_SIZE_NAME, Vector2(pVRSTexture->GetSize().x, pVRSTexture->GetSize().y));
}

void MVRSTextureRenderWork::Render(const MRenderInfo& info)
{
	const auto pEdgeTexture = GetInputTexture(0);
	if (!pEdgeTexture)
	{
		return;
	}

	const std::shared_ptr<MShaderPropertyBlock>& params = m_pVRSGenerator->GetShaderPropertyBlock(0);

	const Vector2i n2ThreadNum = {
		pEdgeTexture->GetSize2D().x / m_n2TexelSize.x + (pEdgeTexture->GetSize2D().x % m_n2TexelSize.x != 0),
		pEdgeTexture->GetSize2D().y / m_n2TexelSize.y + (pEdgeTexture->GetSize2D().y % m_n2TexelSize.y != 0)
	};

	auto pVRSTexture = GetRenderOutput(0)->GetTexture();

	params->SetTexture(MShaderPropertyName::VRS_EDGE_TEXTURE_NAME, pEdgeTexture);

	info.pPrimaryRenderCommand->AddRenderToTextureBarrier({ pEdgeTexture.get() }, METextureBarrierStage::EComputeShaderRead);

	info.pPrimaryRenderCommand->AddRenderToTextureBarrier({ pVRSTexture.get() }, METextureBarrierStage::EComputeShaderWrite);

	info.pPrimaryRenderCommand->DispatchComputeJob(m_pVRSGenerator, n2ThreadNum.x, n2ThreadNum.y, 1);
}

void MVRSTextureRenderWork::BindTarget()
{
	if (auto params = m_pVRSGenerator->GetShaderPropertyBlock(0))
	{
		params->SetTexture(MShaderPropertyName::VRS_OUTPUT_VRS_TEXTURE_NAME, GetRenderOutput(0)->GetTexture());
	}
}

std::vector<MRenderTaskInputDesc> MVRSTextureRenderWork::GetInputName()
{
	return {
		{ MEdgeDetectionRenderWork::EdgeDetectionResult, METextureBarrierStage::EPixelShaderSample },
	};
}

std::vector<MRenderTaskOutputDesc> MVRSTextureRenderWork::GetOutputName()
{
    return {
		{ VRS_TEXTURE, {false, MColor::Black_T }},
	};
}
