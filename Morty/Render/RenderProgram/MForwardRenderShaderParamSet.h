/**
 * @File         MForwardRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFORWARD_RENDER_SHADER_PARAM_SET_H_
#define _M_MFORWARD_RENDER_SHADER_PARAM_SET_H_
#include "Utility/MGlobal.h"
#include "MRenderInfo.h"
#include "Material/MShaderParamSet.h"


class MEngine;
class MORTY_API MForwardRenderShaderParamSet : public MShaderParamSet
{
public:

	MForwardRenderShaderParamSet();
	virtual ~MForwardRenderShaderParamSet();

public:

	virtual void InitializeShaderParamSet(MEngine* pEngine);
	virtual void ReleaseShaderParamSet(MEngine* pEngine);


	void UpdateShaderSharedParams(MRenderInfo& info);

	void SetShadowMapTexture(MTexture* pTexture);
	void SetBrdfMapTexture(MTexture* pTexture);

public:
	/*_M_E_cbWorldMatrix*/
	MShaderConstantParam* m_pWorldMatrixParam;
	/*_M_E_cbWorldInfo*/
	MShaderConstantParam* m_pWorldInfoParam;
	/*_M_E_cbLights*/
	MShaderConstantParam* m_pLightInfoParam;
	/*_M_E_cbShadowInfo*/
	MShaderConstantParam* m_pShadowInfoParam;

	MShaderTextureParam* m_pShadowTextureParam;
	MShaderTextureParam* m_pDiffuseMapTextureParam;
	MShaderTextureParam* m_pSpecularMapTextureParam;
	MShaderTextureParam* m_pBrdfMapTextureParam;
};


class MForwardRenderTransparentShaderParamSet : public MForwardRenderShaderParamSet
{
public:
	MForwardRenderTransparentShaderParamSet();
	virtual ~MForwardRenderTransparentShaderParamSet();


	virtual void InitializeShaderParamSet(MEngine* pEngine) override;
	virtual void ReleaseShaderParamSet(MEngine* pEngine) override;

public:

	MShaderTextureParam* m_pTransparentFrontTextureParam;
	MShaderTextureParam* m_pTransparentBackTextureParam;
};


#endif
