/**
 * @File         MMaterialTemplateResource
 * 
 * @Created      2019-09-01 15:25:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MMaterialTemplate.h"

namespace morty
{

class MORTY_API MMaterialTemplateResource : public MMaterialTemplate
{
public:
    MORTY_CLASS(MMaterialTemplateResource);

    std::shared_ptr<MMaterialTemplate> GetMaterial() const;

    bool                               Load(std::unique_ptr<MResourceData>&& pResourceData) override;

    bool                               SaveTo(std::unique_ptr<MResourceData>& pResourceData) override;

private:
    std::unique_ptr<MResourceData> m_resourceData = nullptr;
};

}// namespace morty