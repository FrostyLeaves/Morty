/**
 * @File         MForwardRenderShaderPropertyBlock
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
class MORTY_API MForwardRenderShaderPropertyBlock
{
public:

	MForwardRenderShaderPropertyBlock();
	virtual ~MForwardRenderShaderPropertyBlock();

public:

	virtual void BindMaterial(const std::shared_ptr<MMaterial>& pMaterial);
	virtual void ReleaseShaderParamSet(MEngine* pEngine);


	const std::shared_ptr<MShaderPropertyBlock>& GetShaderPropertyBlock() const { return m_pShaderPropertyBlock; }

	void UpdateShaderSharedParams(MRenderInfo& info);

	void SetShadowMapTexture(MTexture* pTexture);
	void SetBrdfMapTexture(MTexture* pTexture);

public:
	/*cbSceneMatrix*/
	std::shared_ptr<MShaderConstantParam> m_pWorldMatrixParam = nullptr;
	/*cbSceneInformation*/
	std::shared_ptr<MShaderConstantParam> m_pWorldInfoParam = nullptr;
	/*cbLightInformation*/
	std::shared_ptr<MShaderConstantParam> m_pLightInfoParam = nullptr;
	/*cbShadowInformation*/
	std::shared_ptr<MShaderConstantParam> m_pShadowInfoParam = nullptr;

	std::shared_ptr<MShaderTextureParam> m_pShadowTextureParam = nullptr;
	std::shared_ptr<MShaderTextureParam> m_pDiffuseMapTextureParam = nullptr;
	std::shared_ptr<MShaderTextureParam> m_pSpecularMapTextureParam = nullptr;
	std::shared_ptr<MShaderTextureParam> m_pBrdfMapTextureParam = nullptr;

protected:
	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
};


class MForwardRenderTransparentShaderPropertyBlock : public MForwardRenderShaderPropertyBlock
{
public:
	MForwardRenderTransparentShaderPropertyBlock();
	virtual ~MForwardRenderTransparentShaderPropertyBlock();


	virtual void BindMaterial(const std::shared_ptr<MMaterial>& pMaterial) override;
	virtual void ReleaseShaderParamSet(MEngine* pEngine) override;

public:

	std::shared_ptr<MShaderTextureParam> m_pTransparentFrontTextureParam = nullptr;
	std::shared_ptr<MShaderTextureParam> m_pTransparentBackTextureParam = nullptr;
};


#endif
