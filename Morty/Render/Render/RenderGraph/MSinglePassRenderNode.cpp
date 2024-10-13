#include "RenderProgram/RenderGraph/MSinglePassRenderNode.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "MRenderCommon.h"
#include "MVRSTextureRenderNode.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "Mesh/MVertex.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Scene/MScene.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MBounds.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(ISinglePassRenderNode, MRenderTaskNode)

void ISinglePassRenderNode::Initialize(MEngine* pEngine)
{
    m_engine = pEngine;

#if MORTY_DEBUG
    m_renderPass.m_strDebugName = GetTypeName();
#endif
}

void ISinglePassRenderNode::Release()
{
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

std::vector<std::shared_ptr<MTexture>> ISinglePassRenderNode::GetBackTextures() const
{
    return m_renderPass.GetBackTextures();
}

std::shared_ptr<MTexture> ISinglePassRenderNode::GetDepthTexture() const { return m_renderPass.GetDepthTexture(); }

std::shared_ptr<IGetTextureAdapter> ISinglePassRenderNode::CreateOutput() const
{
    auto pOutput = std::make_shared<MGetTextureAdapter>(m_renderPass.GetBackTexture(0));

    return pOutput;
}

MRenderTargetGroup ISinglePassRenderNode::AutoBindTarget()
{
    MRenderTargetGroup group;

    auto               vOutputDesc = InitOutputDesc();
    MORTY_ASSERT(vOutputDesc.size() == GetOutputSize());

    for (size_t nIdx = 0; nIdx < GetOutputSize(); ++nIdx)
    {
        auto pTexture = GetRenderOutput(nIdx)->GetTexture();

        if (pTexture->GetWriteUsage() & METextureWriteUsageBit::ERenderBack ||
            pTexture->GetWriteUsage() & METextureWriteUsageBit::ERenderPresent)
        {
            group.backTargets.push_back({pTexture, vOutputDesc[nIdx].renderDesc});
        }
        else if (pTexture->GetWriteUsage() & METextureWriteUsageBit::ERenderDepth)
        {
            MORTY_ASSERT(group.depthTarget.pTexture == nullptr);
            group.depthTarget = {pTexture, vOutputDesc[nIdx].renderDesc};
        }
        else { MORTY_ASSERT(false); }
    }

    return group;
}

MRenderTargetGroup ISinglePassRenderNode::AutoBindTargetWithVRS()
{
    auto group = AutoBindTarget();

    auto pVRSTexture  = GetRenderTargetManager()->FindRenderTexture(MVRSTextureRenderNode::VRS_TEXTURE);
    group.shadingRate = {pVRSTexture, {false, MColor::Black_T}};

    return group;
}

void ISinglePassRenderNode::AutoBindBarrierTexture()
{
    m_barrierTexture.clear();
    auto vInputs = InitInputDesc();
    for (size_t nInputIdx = 0; nInputIdx < GetInputSize(); ++nInputIdx)
    {
        m_barrierTexture[vInputs[nInputIdx].barrier].push_back(GetInputTexture(nInputIdx).get());
    }
}

void ISinglePassRenderNode::AutoSetTextureBarrier(MIRenderCommand* pCommand)
{
    for (const auto& [barrier, textures]: m_barrierTexture)
    {
        if (barrier != METextureBarrierStage::EUnknow) { pCommand->AddRenderToTextureBarrier(textures, barrier); }
    }
}

void ISinglePassRenderNode::Resize(Vector2i size)
{
    auto* pRenderSystem = m_engine->FindSystem<MRenderSystem>();

    if (m_renderPass.GetFrameBufferSize() != size)
    {
        m_renderPass.Resize(pRenderSystem->GetDevice());

        //MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
        //pRenderSystem->ResizeFrameBuffer(m_renderPass, size);
    }
}

void ISinglePassRenderNode::SetRenderTarget(const MRenderTargetGroup& renderTarget)
{
    m_renderPass.SetRenderTarget(renderTarget);

    auto* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
    m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
    m_renderPass.GenerateBuffer(pRenderSystem->GetDevice());
}
