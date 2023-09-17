/**
 * @File         MSpecificMaterialIndirectRenderable
 * 
 * @Created      2023-09-01 22:50:05
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MIndexedIndirectRenderable.h"


class MORTY_API MSpecificMaterialIndirectRenderable : public MIndexedIndirectRenderable
{
public:

    void SetMaterial(std::shared_ptr<MMaterial> pMaterial)
    {
        m_pMaterial = pMaterial;
    }

	//override to use other material.
	const std::shared_ptr<MMaterial>& GetMaterial(const MMaterialCullingGroup& group) const override
    {
        MORTY_UNUSED(group);
        return m_pMaterial;
    }


private:
    std::shared_ptr<MMaterial> m_pMaterial = nullptr;
};
