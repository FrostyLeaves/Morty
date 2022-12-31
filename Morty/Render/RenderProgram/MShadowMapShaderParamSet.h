/**
 * @File         MShadowMapShaderPropertyBlock
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#ifndef _M_SHADOW_MAP_SHADER_PROPERTY_BLOCK_H_
#define _M_SHADOW_MAP_SHADER_PROPERTY_BLOCK_H_
#include "Utility/MGlobal.h"
#include "MRenderInfo.h"
#include "Material/MShaderParamSet.h"


class MEngine;
class MORTY_API MShadowMapShaderPropertyBlock
{
public:

	MShadowMapShaderPropertyBlock();
	virtual ~MShadowMapShaderPropertyBlock();

public:

	void BindMaterial(const std::shared_ptr<MMaterial>& pMaterial);
	void ReleaseShaderParamSet(MEngine* pEngine);

	const std::shared_ptr<MShaderPropertyBlock>& GetShaderPropertyBlock() const { return m_pShaderPropertyBlock; }


	void UpdateShaderSharedParams(MRenderInfo& info) const;

protected:

	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
	std::shared_ptr<MShaderConstantParam> m_pWorldMatrixParam = nullptr;
};


#endif
