#include "MCullingResultSpecificMaterialRenderable.h"
#include "Material/MMaterial.h"

using namespace morty;

MMaterialTemplatePtr MCullingResultSpecificMaterialRenderable::GetMaterial(const MMaterialCullingGroup& group) const
{
    MORTY_UNUSED(group);
    return m_material;
}