/**
 * @File         MFrameShaderPropertyBlock
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MMaterial.h"
#include "Material/MShaderParam.h"
#include "Utility/MGlobal.h"
#include "MRenderInfo.h"
#include "Material/MShaderPropertyBlock.h"
#include "RenderWork/MRenderWork.h"
#include <memory>

class MEngine;
class MORTY_API MFrameShaderPropertyBlock : public IPropertyBlockAdapter
{
public:

	MFrameShaderPropertyBlock() = default;
    ~MFrameShaderPropertyBlock() override = default;

public:

	virtual void Initialize(MEngine* pEngine);
    virtual void Release(MEngine* pEngine);

	virtual std::shared_ptr<MMaterial> LoadMaterial(MEngine* pEngine) const;

	virtual void BindMaterial(const std::shared_ptr<MMaterial>& pMaterial);

	std::shared_ptr<MShaderPropertyBlock> GetPropertyBlock() const override;

	std::shared_ptr<MMaterial> GetMaterial() const { return m_pMaterial; }

	void UpdateShaderSharedParams(MRenderInfo& info);

	void SetShadowMapTexture(std::shared_ptr<MTexture> pTexture);
	void SetBrdfMapTexture(std::shared_ptr<MTexture> pTexture);

public:

	std::shared_ptr<MShaderConstantParam> m_pFrameParam = nullptr;
	std::shared_ptr<MShaderConstantParam> m_pLightParam = nullptr;

	/*u_texShadowMap*/
	std::shared_ptr<MShaderTextureParam> m_pShadowTextureParam = nullptr;
	/*u_texIrradianceMap*/
	std::shared_ptr<MShaderTextureParam> m_pDiffuseMapTextureParam = nullptr;
	/*u_texPrefilterMap*/
	std::shared_ptr<MShaderTextureParam> m_pSpecularMapTextureParam = nullptr;
	/*u_texBrdfLUT*/
	std::shared_ptr<MShaderTextureParam> m_pBrdfMapTextureParam = nullptr;

	/*u_vBonesMatrix*/
	std::shared_ptr<MShaderStorageParam> m_pAnimationBonesParam = nullptr;
	/*u_vBonesOffset*/
	std::shared_ptr<MShaderStorageParam> m_pAnimationOffsetParam = nullptr;

	std::shared_ptr<MShaderStorageParam> m_pRWVoxelTableParam = nullptr;
	std::shared_ptr<MShaderTextureParam> m_pVoxelGITextureParam = nullptr;

protected:
	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;
};


class MORTY_API MForwardRenderTransparentShaderPropertyBlock : public MFrameShaderPropertyBlock
{
public:

	void BindMaterial(const std::shared_ptr<MMaterial>& pMaterial) override;

	std::shared_ptr<MShaderTextureParam> m_pTransparentFrontTextureParam = nullptr;
	std::shared_ptr<MShaderTextureParam> m_pTransparentBackTextureParam = nullptr;
};
