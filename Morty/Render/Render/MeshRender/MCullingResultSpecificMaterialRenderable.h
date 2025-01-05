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
    void                               SetMaterial(const MMaterialTemplatePtr& tMaterials) { m_material = tMaterials; }

    //override to use other material.
    [[nodiscard]] MMaterialTemplatePtr GetMaterial(const MMaterialCullingGroup& group) const override;

private:
    MMaterialTemplatePtr m_material;
};

}// namespace morty