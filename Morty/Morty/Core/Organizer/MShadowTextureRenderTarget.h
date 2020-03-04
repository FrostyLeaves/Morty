/**
 * @File         MShadowTextureRenderTarget
 * 
 * @Created      2020-03-02 16:44:59
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSHADOWTEXTURERENDERTARGET_H_
#define _M_MSHADOWTEXTURERENDERTARGET_H_
#include "MGlobal.h"
#include "MTextureRenderTarget.h"

#include "MObject.h"

class MScene;
class MStruct;
class MShaderParam;
class MMaterial;
class MORTY_CLASS MShadowTextureRenderTarget : public MObject, public MTextureRenderTarget
{
public:
    M_OBJECT(MShadowTextureRenderTarget)
public:
    MShadowTextureRenderTarget();
    virtual ~MShadowTextureRenderTarget();

public:

    virtual void OnCreated() override;

    virtual void OnRender(MIRenderer* pRenderer) override;

    void SetScene(MScene* pScene) { m_pScene = pScene; }

private:

    MScene* m_pScene;

private:

	MMaterial* m_pStaticMaterial;
	MMaterial* m_pAnimMaterial;

    MShaderParam* m_pStaticMeshParam;
    MShaderParam* m_pAnimMeshParam;

	MShaderParam* m_pStaticWorldParam;
	MShaderParam* m_pAnimWorldParam;

    MShaderParam* m_pAnimBonesParam;
};


#endif
