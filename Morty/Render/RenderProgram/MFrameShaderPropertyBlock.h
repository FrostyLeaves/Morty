/**
 * @File         MFrameShaderPropertyBlock
 * 
 * @Created      2020-07-2 11:45:49
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MMaterial.h"
#include "Shader/MShaderParam.h"
#include "Utility/MGlobal.h"
#include "MRenderInfo.h"
#include "Shader/MShaderPropertyBlock.h"
#include "RenderWork/MRenderWork.h"
#include <memory>


class MORTY_API MFramePropertyDecorator : public IShaderPropertyUpdateDecorator
{
public:
	void BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock) override;
	void Update(const MRenderInfo& info) override;

	std::shared_ptr<MShaderConstantParam> m_pFrameParam = nullptr;
};

class MORTY_API MLightPropertyDecorator : public IShaderPropertyUpdateDecorator
{
public:
	void BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock) override;
	void Update(const MRenderInfo& info) override;

	std::shared_ptr<MShaderConstantParam> m_pLightParam = nullptr;
	std::shared_ptr<MShaderTextureParam> m_pDiffuseMapTextureParam = nullptr;
	std::shared_ptr<MShaderTextureParam> m_pSpecularMapTextureParam = nullptr;
};

class MORTY_API MAnimationPropertyDecorator : public IShaderPropertyUpdateDecorator
{
public:
	void BindMaterial(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock) override;
	void Update(const MRenderInfo& info) override;

	std::shared_ptr<MShaderStorageParam> m_pAnimationBonesParam = nullptr;
	std::shared_ptr<MShaderStorageParam> m_pAnimationOffsetParam = nullptr;

};

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

	void RegisterPropertyDecorator(const std::shared_ptr<IShaderPropertyUpdateDecorator>& pDecorator);

	void SetBrdfMapTexture(std::shared_ptr<MTexture> pTexture);

public:

	/*u_texBrdfLUT*/
	std::shared_ptr<MShaderTextureParam> m_pBrdfMapTextureParam = nullptr;

protected:
	std::shared_ptr<MMaterial> m_pMaterial = nullptr;
	std::shared_ptr<MShaderPropertyBlock> m_pShaderPropertyBlock = nullptr;

	std::vector<std::shared_ptr<IShaderPropertyUpdateDecorator>> m_vPropertyUpdateDecorator;
};
