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

void MBasicPostProcessRenderNode::Execute(const MRenderInfo& info, IRenderCommand* primaryCommand)
{
    MORTY_UNUSED(info);

    MIMesh*        pScreenMesh = info.scene->GetManager<MMeshManager>()->GetScreenRect();
    const Vector2i size        = m_renderPass.GetFrameBufferSize();
    MRenderPassCmd command     = primaryCommand->BeginRenderPass(&m_renderPass);

    command.SetViewportAndScissor(MSetViewportCmd{.rect = MRecti(0, 0, size.x, size.y)});

    command.SetMaterial(m_material.get(), m_material->GetTemplate()->GetDefaultPass());
    //command.SetShaderParameterSet(GetRenderGraph()->GetFrameProperty()->GetParameterSet());
    command.DrawMesh(pScreenMesh);

    primaryCommand->EndRenderPass(command);
}

void MBasicPostProcessRenderNode::BindInOutTexture()
{
    if (auto pParameterSet = m_material->GetMaterialParameterSet())
    {
        for (size_t nInputIdx = 0; nInputIdx < GetInputSize(); ++nInputIdx)
        {
            pParameterSet->SetTexture(
                    MShaderPropertyName::POSTPROCESS_SCREEN_TEXTURE[nInputIdx],
                    GetInputTexture(nInputIdx)
            );
        }
    }

    Super::BindInOutTexture();
}
