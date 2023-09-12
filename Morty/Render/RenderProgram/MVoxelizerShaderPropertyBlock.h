/**
 * @File         MVoxelizerShaderPropertyBlock
 * 
 * @Created      2023-09-01 22:58:32
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Material/MShaderParam.h"
#include "Utility/MGlobal.h"
#include "MRenderInfo.h"
#include "Material/MShaderPropertyBlock.h"
#include "MForwardRenderShaderPropertyBlock.h"
#include "Render/MBuffer.h"

struct MVoxelMapSetting
{
    Vector3 f3VoxelOrigin;
    float fResolution;
    float fVoxelStep;
};


class MORTY_API MVoxelizerShaderPropertyBlock : public MForwardRenderShaderPropertyBlock
{
public:

	MVoxelizerShaderPropertyBlock() = default;
	~MVoxelizerShaderPropertyBlock() override = default;

	void Initialize(MEngine* pEngine) override;
    void Release(MEngine* pEngine) override;

	std::shared_ptr<MMaterial> LoadMaterial(MEngine* pEngine) const override;
	void BindMaterial(const std::shared_ptr<MMaterial>& pMaterial) override;

	void SetVoxelMapSetting(const MVoxelMapSetting setting);

protected:

	std::shared_ptr<MShaderStorageParam> m_pRWVoxelTableParam = nullptr;
	std::shared_ptr<MShaderConstantParam> m_pVoxelMapSetting = nullptr;

	MBuffer m_rwVoxelTableBuffer;
};