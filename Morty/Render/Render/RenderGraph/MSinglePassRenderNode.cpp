#include "Render/RenderGraph/MSinglePassRenderNode.h"
#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Component/MCameraComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"
#include "Engine/MEngine.h"
#include "MRenderCommon.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "Mesh/MVertex.h"
#include "Model/MSkeleton.h"
#include "RHI/Abstract/MIDevice.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "Utility/MBounds.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(ISinglePassRenderNode, MRenderTaskNode)

ISinglePassRenderNode::ISinglePassRenderNode()
{
#if MORTY_DEBUG
    m_renderPass.m_strDebugName = GetTypeName().ToString();
#endif
}

void ISinglePassRenderNode::Release()
{
    auto* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

MRenderTargetGroup ISinglePassRenderNode::AutoBindTarget()
{
    MRenderTargetGroup group;

    for (size_t nIdx = 0; nIdx < GetOutputSize(); ++nIdx)
    {
        auto pOutput  = GetRenderOutput(nIdx);
        auto pTexture = pOutput->GetRenderTexture();

        if (pTexture->GetWriteUsage() & METextureWriteUsageBit::ERenderBack ||
            pTexture->GetWriteUsage() & METextureWriteUsageBit::ERenderPresent)
        {
            group.backTargets.emplace_back(pOutput->CreateRenderTarget());
        }
        else if (pTexture->GetWriteUsage() & METextureWriteUsageBit::ERenderDepth)
        {
            MORTY_ASSERT(group.depthTarget.pTexture == nullptr);
            group.depthTarget = {pOutput->CreateRenderTarget()};
        }
        else { MORTY_ASSERT(false); }
    }

    return group;
}

MRenderTargetGroup ISinglePassRenderNode::AutoBindTargetWithVRS()
{
    auto group = AutoBindTarget();

    auto pVRSTexture  = GetRenderGraph()->GetTextureVRS();
    group.shadingRate = {pVRSTexture, {false, MColor::Black_T}};

    return group;
}

void ISinglePassRenderNode::Resize(Vector2i size)
{
    Super::Resize(size);

    auto* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();

    if (m_renderPass.GetFrameBufferSize() != size) { m_renderPass.Resize(pRenderSystem->GetDevice()); }
}

void ISinglePassRenderNode::SetRenderTarget(const MRenderTargetGroup& renderTarget)
{
    m_renderPass.SetRenderTarget(renderTarget);

    auto* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
    m_renderPass.GenerateBuffer(pRenderSystem->GetDevice());
}

void ISinglePassRenderNode::BindInOutTexture()
{
    Super::AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTarget());
};