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
#include "Render/MBuffer.h"
#include "RenderProgram/MRenderInfo.h"
#include "Material/MShaderParamSet.h"
#include "RenderProgram/RenderWork/MRenderWork.h"



class MEngine;
class MORTY_API MShadowMapShaderPropertyBlock
	: public IPropertyBlockAdapter
{
public:

	void Initialize(MEngine* pEngine);
	void Release(MEngine* pEngine);
	void BindMaterial(const std::shared_ptr<MMaterial>& pMaterial);

	std::vector<std::shared_ptr<MShaderPropertyBlock>> GetPropertyBlock() const override;
	void UpdateShaderSharedParams(MRenderInfo& info) ;


protected:

	MEngine* m_pEngine = nullptr;
    std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
	std::shared_ptr<MShaderConstantParam> m_pWorldMatrixParam = nullptr;

	size_t m_nInstanceNum = 0;
};

#endif
