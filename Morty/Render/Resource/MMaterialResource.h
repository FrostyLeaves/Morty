/**
 * @File         MMaterialResource
 * 
 * @Created      2019-09-01 15:25:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MMaterial.h"

namespace morty
{

class MORTY_API MMaterialResource : public MMaterial
{
public:
    MORTY_CLASS(MMaterialResource);

    std::shared_ptr<MMaterial>                GetMaterial() const;


    static std::shared_ptr<MMaterialResource> CreateMaterial(const std::shared_ptr<MResource>& pMaterialTemplate);


    bool                                      Load(std::unique_ptr<MResourceData>&& pResourceData) override;

    bool                                      SaveTo(std::unique_ptr<MResourceData>& pResourceData) override;

private:
    std::unique_ptr<MResourceData> m_resourceData = nullptr;
};

}// namespace morty