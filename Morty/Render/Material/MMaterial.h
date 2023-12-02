/**
 * @File         MMaterial
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"
#include "Resource/MTextureResource.h"

#include "MShaderMacro.h"
#include "MShaderProgram.h"
#include "MShaderBuffer.h"
#include "Material/MShaderPropertyBlock.h"

#include <vector>


enum class MECullMode
{
	EWireframe = 0,
	ECullNone,
	ECullBack,
	ECullFront,

	ERasterizerEnd
};

enum class MEMaterialType
{
	EDefault = 0,
	EDepthPeel,
	ETransparentBlend,
	EOutline,
	EImGui,
	EDeferred,
	ECustom,

	EMaterialTypeEnd,
};

class MShader;
class MShaderResource;
class MORTY_API MMaterial : public MResource
{
public:
	MORTY_CLASS(MMaterial)
    MMaterial() = default;
    ~MMaterial() override = default;
	
	std::vector<std::shared_ptr<MShaderConstantParam>>& GetShaderParams();
	std::vector<std::shared_ptr<MShaderSampleParam>>& GetSampleParams();
	std::vector<std::shared_ptr<MShaderTextureParam>>& GetTextureParams();

	auto& GetShaderPropertyBlocks() { return m_pShaderProgram->GetShaderPropertyBlocks(); }
	const auto& GetShaderPropertyBlocks() const { return m_pShaderProgram->GetShaderPropertyBlocks(); }
	std::shared_ptr<MShaderPropertyBlock> GetMaterialPropertyBlock() const { return m_pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]; }
	std::shared_ptr<MShaderPropertyBlock> GetFramePropertyBlock() const { return m_pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_FRAME]; }
	std::shared_ptr<MShaderPropertyBlock> GetMeshPropertyBlock() const { return m_pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_MESH]; }

	std::shared_ptr<MShaderConstantParam> FindShaderParam(const MStringId& strName);
	std::shared_ptr<MShaderSampleParam> FindSample(const MStringId& strName);
	std::shared_ptr<MShaderTextureParam> FindTexture(const MStringId& strName);

	void SetCullMode(const MECullMode& eType);
	MECullMode GetCullMode() const { return m_eCullMode; }

	void SetMaterialType(const MEMaterialType& eType);
	MEMaterialType GetMaterialType() const { return m_eMaterialType; }

	bool GetConservativeRasterizationEnable() const { return m_bConservativeRasterizationEnable; }
	void SetConservativeRasterizationEnable(bool bEnable) { m_bConservativeRasterizationEnable = bEnable; }

	void SetTexture(const MStringId& strName, std::shared_ptr<MResource> pTexResource);

	bool LoadShader(std::shared_ptr<MResource> pResource);
	bool LoadShader(const MString& strResource);

	void SetShaderMacro(const MShaderMacro& macro);
	MShaderMacro& GetShaderMacro() const { return m_pShaderProgram->GetShaderMacro(); }
	const std::shared_ptr<MShaderProgram>& GetShaderProgram() const { return m_pShaderProgram; }
	
public:

	void OnCreated() override;
	void OnDelete() override;

	void Unload();

#if MORTY_DEBUG
	const char* GetDebugName() const;
#endif

private:

	std::shared_ptr<MShaderProgram> m_pShaderProgram = nullptr;

	MEMaterialType m_eMaterialType = MEMaterialType::EDefault;
	MECullMode m_eCullMode = MECullMode::ECullBack;
	bool m_bConservativeRasterizationEnable = false;
};
