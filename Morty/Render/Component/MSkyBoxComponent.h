/**
 * @File         MSkyBoxComponent
 * 
 * @Created      2022-01-01 21:44:08
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MRenderGlobal.h"
#include "Component/MComponent.h"

#include "Resource/MResource.h"

namespace morty
{

class MTexture;
class MMaterial;
class MORTY_API MSkyBoxComponent : public MComponent
{
public:
    MORTY_CLASS(MSkyBoxComponent)

public:
    MSkyBoxComponent();

    virtual ~MSkyBoxComponent();

public:
    void                       LoadSkyBoxResource(std::shared_ptr<MResource> pTexture);

    std::shared_ptr<MResource> GetSkyBoxResource();

    void                       LoadDiffuseEnvResource(std::shared_ptr<MResource> pTexture);

    void                       LoadSpecularEnvResource(std::shared_ptr<MResource> pTexture);

    std::shared_ptr<MResource> GetDiffuseEnvResource();

    MTexturePtr                GetDiffuseTexture();

    std::shared_ptr<MResource> GetSpecularEnvResource();

    MTexturePtr                GetSpecularTexture();


private:
    MResourceRef               m_Texture;
    MResourceRef               m_DiffuseEnvTexture;
    MResourceRef               m_SpecularEnvTexture;

    std::shared_ptr<MMaterial> m_material;
};

}// namespace morty