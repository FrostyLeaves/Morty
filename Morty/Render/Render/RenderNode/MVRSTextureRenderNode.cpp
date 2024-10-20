#include "MVRSTextureRenderNode.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MEdgeDetectionRenderNode.h"
#include "Material/MMaterial.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Scene/MScene.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Mesh/MVertex.h"

#include "Mesh/MMeshManager.h"
#include "System/MObjectSystem.h"
#include "Utility/MBounds.h"

#include "Material/MComputeDispatcher.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MVRSTextureRenderNode, MRenderTaskNode)


void MVRSTextureRenderNode::Initialize(MEngine* pEngine)
{
    m_engine = pEngine;

    auto           pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
    MRenderSystem* pRenderSystem = pEngine->FindSystem<MRenderSystem>();

    m_texelSize = pRenderSystem->GetDevice()->GetShadingRateTextureTexelSize();

    m_vRSGenerator = pObjectSystem->CreateObject<MComputeDispatcher>();
    m_vRSGenerator->GetShaderProgram()->GetShaderMacro().AddUnionMacro(
            MStringId("MORTY_SHADING_RATE_TEXEL_SIZE"),
            MStringUtil::ToString(m_texelSize.x)
    );
    m_vRSGenerator->LoadComputeShader("Shader/VRS/vrs_image_from_edge_detection.mcs");

    const std::shared_ptr<MShaderPropertyBlock>& params = m_vRSGenerator->GetShaderPropertyBlock(0);

    params->SetValue(MShaderPropertyName::VRS_TEXEL_SIZE_NAME, Vector2(m_texelSize.x, m_texelSize.y));
    params->SetValue(MShaderPropertyName::VRS_EDGE_THRESHOLD_NAME, Vector2(1.0f, 10.0f));
}

void MVRSTextureRenderNode::Release()
{
    m_vRSGenerator->DeleteLater();
    m_vRSGenerator = nullptr;
}

void MVRSTextureRenderNode::Resize(Vector2i size)
{
    MORTY_UNUSED(size);

    const auto                                   pVRSTexture = GetRenderOutput(0)->GetTexture();

    const std::shared_ptr<MShaderPropertyBlock>& params = m_vRSGenerator->GetShaderPropertyBlock(0);
    params->SetValue(
            MShaderPropertyName::VRS_EDGE_TEXTURE_SIZE_NAME,
            Vector2(pVRSTexture->GetSize().x, pVRSTexture->GetSize().y)
    );
}

void MVRSTextureRenderNode::Render(const MRenderInfo& info)
{
    const auto pEdgeTexture = GetInputTexture(0);
    if (!pEdgeTexture) { return; }

    const std::shared_ptr<MShaderPropertyBlock>& params = m_vRSGenerator->GetShaderPropertyBlock(0);

    const Vector2i                               n2ThreadNum = {
            pEdgeTexture->GetSize2D().x / m_texelSize.x + (pEdgeTexture->GetSize2D().x % m_texelSize.x != 0),
            pEdgeTexture->GetSize2D().y / m_texelSize.y + (pEdgeTexture->GetSize2D().y % m_texelSize.y != 0)
    };

    auto pVRSTexture = GetRenderOutput(0)->GetTexture();

    params->SetTexture(MShaderPropertyName::VRS_EDGE_TEXTURE_NAME, pEdgeTexture);

    info.pPrimaryRenderCommand->AddRenderToTextureBarrier(
            {pEdgeTexture.get()},
            METextureBarrierStage::EComputeShaderRead
    );

    info.pPrimaryRenderCommand->AddRenderToTextureBarrier(
            {pVRSTexture.get()},
            METextureBarrierStage::EComputeShaderWrite
    );

    info.pPrimaryRenderCommand->DispatchComputeJob(m_vRSGenerator, n2ThreadNum.x, n2ThreadNum.y, 1);
}

void MVRSTextureRenderNode::BindTarget()
{
    if (auto params = m_vRSGenerator->GetShaderPropertyBlock(0))
    {
        params->SetTexture(MShaderPropertyName::VRS_OUTPUT_VRS_TEXTURE_NAME, GetRenderOutput(0)->GetTexture());
    }
}

std::vector<MRenderTaskInputDesc> MVRSTextureRenderNode::InitInputDesc()
{
    return {
            {MEdgeDetectionRenderNode::EdgeDetectionResult,
             METextureFormat::UNorm_RGBA8,
             METextureBarrierStage::EPixelShaderSample},
    };
}

std::vector<MRenderTaskOutputDesc> MVRSTextureRenderNode::InitOutputDesc()
{
    return {
            {MRenderGraphName::VRS_TEXTURE, METextureFormat::UInt_R8, {false, MColor::Black_T}},
    };
}
