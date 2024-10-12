#include "MSkyBoxRenderable.h"

#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "RHI/MRenderCommand.h"
#include "Scene/MScene.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSkyBoxComponent.h"

#include "Manager/MEnvironmentManager.h"
#include "Mesh/MMeshManager.h"

using namespace morty;

void MSkyBoxRenderable::SetMesh(MIMesh* pMesh) { m_mesh = pMesh; }

void MSkyBoxRenderable::SetMaterial(const std::shared_ptr<MMaterial>& pMaterial) { m_material = pMaterial; }

void MSkyBoxRenderable::SetPropertyBlockAdapter(const std::vector<std::shared_ptr<IPropertyBlockAdapter>>& vAdapter)
{
    m_framePropertyAdapter = vAdapter;
}

void MSkyBoxRenderable::Render(MIRenderCommand* pCommand)
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

    pCommand->SetUseMaterial(m_material);

    for (const auto& pAdapter: m_framePropertyAdapter)
    {
        pCommand->SetShaderPropertyBlock(pAdapter->GetPropertyBlock());
    }

    pCommand->DrawMesh(m_mesh);
}
