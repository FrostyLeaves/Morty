/**
 * @File         MCullingResultSpecificMaterialRenderable
 * 
 * @Created      2023-09-01 22:50:05
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MCullingResultRenderable.h"

namespace morty
{

class MORTY_API MCullingResultSpecificMaterialRenderable : public MCullingResultRenderable
{
public:
    void SetMaterial(std::unordered_map<MStringId, std::shared_ptr<MMaterial>> tMaterials) { m_materials = tMaterials; }

    //override to use other material.
    std::shared_ptr<MMaterial> GetMaterial(const MMaterialCullingGroup& group) const override;

private:
    std::unordered_map<MStringId, std::shared_ptr<MMaterial>> m_materials;
};

}// namespace morty