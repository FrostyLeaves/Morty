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

#include <vector>

#include "MObject.h"

class MStruct;
class MMaterial;
class MShaderConstantParam;
class MIMeshInstance;
class MSkeletonInstance;

struct MShadowRenderGroup
{
	MShadowRenderGroup() :pSkeletonInstance(nullptr) {}
	MSkeletonInstance* pSkeletonInstance;
	std::vector<MIMeshInstance*> vMeshInstances;
};


class MORTY_CLASS MShadowTextureRenderTarget : public MTextureRenderTarget
{
public:
    M_OBJECT(MShadowTextureRenderTarget)

	

public:
    MShadowTextureRenderTarget();
    virtual ~MShadowTextureRenderTarget();

public:

	void Render(MIRenderer* pRenderer, const Matrix4& m4InvProj, std::vector<MShadowRenderGroup>* pGroup);

    virtual void OnCreated() override;
	virtual void OnDelete() override;

    virtual void OnRender(MIRenderer* pRenderer) override;

    virtual void InitRenderPass() override;

private:

	Matrix4 m_m4LightInvProj;
	std::vector<MShadowRenderGroup>* m_pShadowRenderGroup;

	MMaterial* m_pStaticMaterial;
	MMaterial* m_pAnimMaterial;

    MShaderConstantParam* m_pMeshParam;
	MShaderConstantParam* m_pWorldParam;
    MShaderConstantParam* m_pAnimBonesParam;

};


#endif
