/**
 * @File         MShadowMapShaderParamSet
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_SHADOW_MAP_SHADER_PARAM_SET_H_
#define _M_SHADOW_MAP_SHADER_PARAM_SET_H_
#include "Utility/MGlobal.h"
#include "MRenderInfo.h"
#include "Material/MShaderParamSet.h"


class MEngine;
class MORTY_API MShadowMapShaderParamSet : public MShaderPropertyBlock
{
public:

	MShadowMapShaderParamSet();
	virtual ~MShadowMapShaderParamSet();

public:

	virtual void InitializeShaderParamSet(MEngine* pEngine);
	virtual void ReleaseShaderParamSet(MEngine* pEngine);


	void UpdateShaderSharedParams(MRenderInfo& info);

public:

	std::shared_ptr<MShaderConstantParam> m_pWorldMatrixParam;
};


#endif
