/**
 * @File         MSkyBoxComponent
 * 
 * @Created      2022-01-01 21:44:08
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSKYBOXCOMPONENT_H_
#define _M_MSKYBOXCOMPONENT_H_
#include "Utility/MGlobal.h"
#include "Component/MComponent.h"

#include "Resource/MResource.h"

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

    void LoadSkyBoxResource(std::shared_ptr<MResource> pTexture);
    std::shared_ptr<MResource> GetSkyBoxResource();

    void LoadDiffuseEnvResource(std::shared_ptr<MResource> pTexture);
    void LoadSpecularEnvResource(std::shared_ptr<MResource> pTexture);

    std::shared_ptr<MResource> GetDiffuseEnvResource();
    std::shared_ptr<MTexture> GetDiffuseTexture();

    std::shared_ptr<MResource> GetSpecularEnvResource();
    std::shared_ptr<MTexture> GetSpecularTexture();


private:

	MResourceRef m_Texture;
	MResourceRef m_DiffuseEnvTexture;
	MResourceRef m_SpecularEnvTexture;
    
    std::shared_ptr<MMaterial> m_pMaterial;
};


#endif
