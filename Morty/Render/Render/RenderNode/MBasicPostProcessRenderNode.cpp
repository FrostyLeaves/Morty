#include "MBasicPostProcessRenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "RHI/MRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(MBasicPostProcessRenderNode, ISinglePassRenderNode)

void MBasicPostProcessRenderNode::Initialize(MEngine* pEngine)
{
    Super::Initialize(pEngine);

    m_material = CreateMaterial();
}

void MBasicPostProcessRenderNode::Release() { Super::Release(); }

void MBasicPostProcessRenderNode::Render(const MRenderInfo& info)
{
    auto    pCommand    = info.pPrimaryRenderCommand;
    MIMesh* pScreenMesh = GetEngine()->FindGlobalObject<MMeshManager>()->GetScreenRect();

    AutoSetTextureBarrier(pCommand);

    pCommand->BeginRenderPass(&m_renderPass);

    const Vector2i n2Size = m_renderPass.GetFrameBufferSize();

    pCommand->SetViewport(MViewportInfo(0.0f, 0.0f, n2Size.x, n2Size.y));
    pCommand->SetScissor(MScissorInfo(0.0f, 0.0f, n2Size.x, n2Size.y));


    if (pCommand->SetUseMaterial(m_material))
    {
        pCommand->SetShaderPropertyBlock(GetRenderGraph()->GetFrameProperty()->GetPropertyBlock());

        pCommand->DrawMesh(pScreenMesh);
    }

    pCommand->EndRenderPass();
}

void MBasicPostProcessRenderNode::BindTarget()
{
    if (std::shared_ptr<MShaderPropertyBlock> pPropertyBlock = m_material->GetMaterialPropertyBlock())
    {
        for (size_t nInputIdx = 0; nInputIdx < GetInputSize(); ++nInputIdx)
        {
            pPropertyBlock->SetTexture(
                    MShaderPropertyName::POSTPROCESS_SCREEN_TEXTURE[nInputIdx],
                    GetInputTexture(nInputIdx)
            );
        }
    }

    AutoBindBarrierTexture();
    SetRenderTarget(AutoBindTarget());
}
