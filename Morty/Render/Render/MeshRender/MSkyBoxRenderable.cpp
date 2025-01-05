#include "MSkyBoxRenderable.h"
#include "Component/MSkyBoxComponent.h"
#include "Engine/MEngine.h"
#include "Manager/MEnvironmentManager.h"
#include "Material/MMaterial.h"
#include "Mesh/MMeshManager.h"
#include "RHI/Command/MRenderPassCmd.h"
#include "RHI/IRenderCommand.h"
#include "Scene/MScene.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

using namespace morty;

void MSkyBoxRenderable::SetMesh(MIMesh* pMesh) { m_mesh = pMesh; }

void MSkyBoxRenderable::SetMaterial(const MMaterialTemplatePtr& pMaterial) { m_material = pMaterial; }

void MSkyBoxRenderable::SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter)
{
    m_framePropertyAdapter = vAdapter;
}

void MSkyBoxRenderable::Render(MRenderPassCmd* pCommand)
{
    if (!m_mesh)
    {
        MORTY_ASSERT(m_mesh);
        return;
    }

    if (!m_material)
    {
        MORTY_ASSERT(m_material);
        return;
    }

    pCommand->SetMaterial(m_material.get());

    for (const auto& pAdapter: m_framePropertyAdapter)
    {
        pCommand->SetShaderPropertyBlock(pAdapter->GetPropertyBlock().get());
    }

    pCommand->DrawMesh(m_mesh);
}
