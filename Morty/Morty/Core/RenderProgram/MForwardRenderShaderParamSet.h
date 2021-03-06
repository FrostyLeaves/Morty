/**
 * @File         MForwardRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_MFORWARD_RENDER_SHADER_PARAM_SET_H_
#define _M_MFORWARD_RENDER_SHADER_PARAM_SET_H_
#include "MGlobal.h"
#include "Shader/MShaderParamSet.h"


#include "MRenderInfo.h"

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

public:

	MShaderConstantParam* m_pWorldMatrixParam;
	MShaderConstantParam* m_pWorldInfoParam;
	MShaderConstantParam* m_pLightInfoParam;

	MShaderSampleParam* m_pDefaultSampleParam;

	MShaderTextureParam* m_pShadowTextureParam;
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
