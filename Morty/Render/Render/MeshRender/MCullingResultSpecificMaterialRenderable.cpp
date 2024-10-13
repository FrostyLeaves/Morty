#include "MCullingResultSpecificMaterialRenderable.h"
#include "Material/MMaterial.h"

using namespace morty;

std::shared_ptr<MMaterial> MCullingResultSpecificMaterialRenderable::GetMaterial(const MMaterialCullingGroup& group
) const
{
    for (const auto& [strDefinedKey, pMaterial]: m_materials)
    {
        if (group.pMaterial->GetShaderMacro().HasMacro(strDefinedKey)) { return pMaterial; }
    }

    return nullptr;
}