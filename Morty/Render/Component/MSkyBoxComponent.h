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

    void LoadTexture(MResource* pTexture);
    MResource* GetTexture();

    void LoadEnvTexutre(MResource* pTexture);
    MResource* GetEnvTexture();

    MTexture* GetEnvironmentTexture();

private:

	MResourceKeeper m_Texture;
	MResourceKeeper m_EnvTexture;
    
    MMaterial* m_pMaterial;
};


#endif
