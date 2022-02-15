/**
 * @File         MSkyBoxComponent
 * 
 * @Created      2022-01-01 21:44:08
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSKYBOXCOMPONENT_H_
#define _M_MSKYBOXCOMPONENT_H_
#include "MGlobal.h"
#include "MComponent.h"

#include "MResource.h"

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

    void LoadSkyBoxResource(MResource* pTexture);
    MResource* GetSkyBoxResource();

    void LoadDiffuseEnvResource(MResource* pTexture);
    void LoadSpecularEnvResource(MResource* pTexture);

    MResource* GetDiffuseEnvResource();
    MTexture* GetDiffuseTexture();

    MResource* GetSpecularEnvResource();
    MTexture* GetSpecularTexture();


private:

	MResourceKeeper m_Texture;
	MResourceKeeper m_DiffuseEnvTexture;
	MResourceKeeper m_SpecularEnvTexture;
    
    MMaterial* m_pMaterial;
};


#endif
