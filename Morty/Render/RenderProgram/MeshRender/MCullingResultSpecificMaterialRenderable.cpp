#include "MCullingResultSpecificMaterialRenderable.h"
#include "Material/MMaterial.h"

std::shared_ptr<MMaterial> MCullingResultSpecificMaterialRenderable::GetMaterial(const MMaterialCullingGroup &group) const 
{
    for (const auto &[strDefinedKey, pMaterial] : m_tMaterials)
    {
        if (group.pMaterial->GetShaderMacro().HasMacro(strDefinedKey))
        {
            return pMaterial;
        }
    }

  return nullptr;
}