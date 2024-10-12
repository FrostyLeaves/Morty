#include "MSinglePassRenderWork.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MVRSTextureRenderWork.h"
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
#include "Utility/MBounds.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(ISinglePassRenderWork, MRenderTaskNode)

void ISinglePassRenderWork::Initialize(MEngine* pEngine)
{
    m_engine = pEngine;

#if MORTY_DEBUG
    m_renderPass.m_strDebugName = GetTypeName();
#endif
}

void ISinglePassRenderWork::Release()
{
    MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
    m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
}

std::vector<std::shared_ptr<MTexture>> ISinglePassRenderWork::GetBackTextures() const
{
    return m_renderPass.GetBackTextures();
}

std::shared_ptr<MTexture> ISinglePassRenderWork::GetDepthTexture() const { return m_renderPass.GetDepthTexture(); }

std::shared_ptr<IGetTextureAdapter> ISinglePassRenderWork::CreateOutput() const
{
    auto pOutput = std::make_shared<MGetTextureAdapter>(m_renderPass.GetBackTexture(0));

    return pOutput;
}

MRenderTargetGroup ISinglePassRenderWork::AutoBindTarget()
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

MRenderTargetGroup ISinglePassRenderWork::AutoBindTargetWithVRS()
{
    auto group = AutoBindTarget();

    auto pVRSTexture  = GetRenderTargetManager()->FindRenderTexture(MVRSTextureRenderWork::VRS_TEXTURE);
    group.shadingRate = {pVRSTexture, {false, MColor::Black_T}};

    return group;
}

void ISinglePassRenderWork::AutoBindBarrierTexture()
{
    m_barrierTexture.clear();
    auto vInputs = InitInputDesc();
    for (size_t nInputIdx = 0; nInputIdx < GetInputSize(); ++nInputIdx)
    {
        m_barrierTexture[vInputs[nInputIdx].barrier].push_back(GetInputTexture(nInputIdx).get());
    }
}

void ISinglePassRenderWork::AutoSetTextureBarrier(MIRenderCommand* pCommand)
{
    for (const auto& [barrier, textures]: m_barrierTexture)
    {
        if (barrier != METextureBarrierStage::EUnknow) { pCommand->AddRenderToTextureBarrier(textures, barrier); }
    }
}

void ISinglePassRenderWork::Resize(Vector2i size)
{
    MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();

    if (m_renderPass.GetFrameBufferSize() != size)
    {
        m_renderPass.Resize(pRenderSystem->GetDevice());

        //MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
        //pRenderSystem->ResizeFrameBuffer(m_renderPass, size);
    }
}

void ISinglePassRenderWork::SetRenderTarget(const MRenderTargetGroup& renderTarget)
{
    m_renderPass.SetRenderTarget(renderTarget);

    MRenderSystem* pRenderSystem = m_engine->FindSystem<MRenderSystem>();
    m_renderPass.DestroyBuffer(pRenderSystem->GetDevice());
    m_renderPass.GenerateBuffer(pRenderSystem->GetDevice());
}
