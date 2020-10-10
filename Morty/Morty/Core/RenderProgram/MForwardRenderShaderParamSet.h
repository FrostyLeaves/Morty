/**
 * @File         MForwardRenderProgram
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       Pobrecito
**/

#ifndef _M_MFORWARD_RENDER_SHADER_PARAM_SET_H_
#define _M_MFORWARD_RENDER_SHADER_PARAM_SET_H_
#include "MGlobal.h"
#include "Shader/MShaderParamSet.h"

class MEngine;
class MORTY_CLASS MForwardRenderShaderParamSet : public MShaderParamSet
{
public:

	MForwardRenderShaderParamSet();
	virtual ~MForwardRenderShaderParamSet();

public:

	void InitializeShaderParamSet(MEngine* pEngine);
	void ReleaseShaderParamSet(MEngine* pEngine);

public:

	MShaderConstantParam* m_pWorldMatrixParam;
	MShaderConstantParam* m_pWorldInfoParam;
	MShaderConstantParam* m_pLightInfoParam;

	MShaderSampleParam* m_pDefaultSampleParam;

	MShaderTextureParam* m_pShadowTextureParam;
	MShaderTextureParam* m_pTransparentFrontTextureParam;
	MShaderTextureParam* m_pTransparentBackTextureParam;
};

#endif
