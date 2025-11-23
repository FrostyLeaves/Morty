#include "MBasicPostProcessRenderNode.h"

#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "MForwardRenderNode.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/IRenderCommand.h"
#include "RHI/MRenderPass.h"
#include "Render/RenderGraph/MRenderGraph.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(MBasicPostProcessRenderNode, ISinglePassRenderNode)

void MBasicPostProcessRenderNode::OnCreated()
{
    Super::OnCreated();

    m_material = CreateMaterial();
}

void MBasicPostProcessRenderNode::Release() { Super::Release(); }

void MBasicPostProcessRenderNode::Render(const MRenderInfo& info)
{
    auto           pCommand    = info.pPrimaryRenderCommand;
    MIMesh*        pScreenMesh = GetEngine()->FindGlobalObject<MMeshManager>()->GetScreenRect();
    const Vector2i n2Size      = m_renderPass.GetFrameBufferSize();
    MRenderPassCmd command     = pCommand->BeginRenderPass(&m_renderPass);

    command.SetViewportAndScissor(MSetViewportCmd{
            .x      = 0.0f,
            .y      = 0.0f,
            .width  = static_cast<float>(n2Size.x),
            .height = static_cast<float>(n2Size.y)
    });

    command.SetMaterial(m_material.get(), m_material->GetTemplate()->GetDefaultPass());
    //command.SetShaderPropertyBlock(GetRenderGraph()->GetFrameProperty()->GetPropertyBlock());
    command.DrawMesh(pScreenMesh);

    pCommand->EndRenderPass(command);
}

void MBasicPostProcessRenderNode::BindInOutTexture()
{
    if (auto pPropertyBlock = m_material->GetMaterialPropertyBlock())
    {
        for (size_t nInputIdx = 0; nInputIdx < GetInputSize(); ++nInputIdx)
        {
            pPropertyBlock->SetTexture(
                    MShaderPropertyName::POSTPROCESS_SCREEN_TEXTURE[nInputIdx],
                    GetInputTexture(nInputIdx)
            );
        }
    }

    Super::BindInOutTexture();
}
