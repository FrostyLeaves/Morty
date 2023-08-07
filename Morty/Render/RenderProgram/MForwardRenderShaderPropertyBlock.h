/**
 * @File         MForwardRenderShaderPropertyBlock
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "MRenderInfo.h"
#include "Material/MShaderPropertyBlock.h"
#include "RenderWork/MRenderWork.h"

class MEngine;
class MORTY_API MForwardRenderShaderPropertyBlock : public IPropertyBlockAdapter
{
public:

	MForwardRenderShaderPropertyBlock() = default;
    ~MForwardRenderShaderPropertyBlock() override = default;

public:

	void Initialize(MEngine* pEngine);
    void Release(MEngine* pEngine);

	virtual void BindMaterial(const std::shared_ptr<MMaterial>& pMaterial);

	std::shared_ptr<MShaderPropertyBlock> GetPropertyBlock() const override;

	void UpdateShaderSharedParams(MRenderInfo& info);

	void SetShadowMapTexture(std::shared_ptr<MTexture> pTexture);
	void SetBrdfMapTexture(std::shared_ptr<MTexture> pTexture);

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
	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
};


class MORTY_API MForwardRenderTransparentShaderPropertyBlock : public MForwardRenderShaderPropertyBlock
{
public:
	MForwardRenderTransparentShaderPropertyBlock() = default;
	virtual ~MForwardRenderTransparentShaderPropertyBlock() = default;


	virtual void BindMaterial(const std::shared_ptr<MMaterial>& pMaterial) override;

public:

	std::shared_ptr<MShaderTextureParam> m_pTransparentFrontTextureParam = nullptr;
	std::shared_ptr<MShaderTextureParam> m_pTransparentBackTextureParam = nullptr;
};
