/**
 * @File         MMaterialTemplate
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Object/MObject.h"
#include "Resource/MTextureResource.h"

#include "Shader/MShaderMacro.h"
#include "Shader/MShaderProgram.h"
#include "Shader/MShaderPropertyBlock.h"

MORTY_SPACE_BEGIN

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
class MORTY_API MMaterialTemplate : public MResource
{
public:
	MORTY_CLASS(MMaterialTemplate)
    MMaterialTemplate() = default;
    ~MMaterialTemplate() override = default;
	
	void SetCullMode(const MECullMode& eType);
	MECullMode GetCullMode() const { return m_eCullMode; }

	void SetMaterialType(const MEMaterialType& eType);
	MEMaterialType GetMaterialType() const { return m_eMaterialType; }

	void SetShaderMacro(const MShaderMacro& macro);
	MShaderMacro GetShaderMacro() const { return m_pShaderProgram->GetShaderMacro(); }
	void AddDefine(const MStringId& strKey, const MString& strValue);

	const std::shared_ptr<MShaderProgram>& GetShaderProgram() const { return m_pShaderProgram; }

	bool GetConservativeRasterizationEnable() const { return m_bConservativeRasterizationEnable; }
	void SetConservativeRasterizationEnable(bool bEnable) { m_bConservativeRasterizationEnable = bEnable; }

	void SetShadingRate(const Vector2i n2ShadingRate);
	Vector2i GetShadingRate() const { return m_n2ShadingRate; }


	bool LoadShader(std::shared_ptr<MResource> pResource);
	bool LoadShader(const MString& strResource);


	static std::shared_ptr<MShaderPropertyBlock> CreateFramePropertyBlock(const std::shared_ptr<MShaderProgram>& pShaderProgram);
	static std::shared_ptr<MShaderPropertyBlock> CreateMeshPropertyBlock(const std::shared_ptr<MShaderProgram>& pShaderProgram);
	static std::shared_ptr<MShaderPropertyBlock> CreateMaterialPropertyBlock(const std::shared_ptr<MShaderProgram>& pShaderProgram);

public:

	void OnCreated() override;
	void OnDelete() override;

private:

	std::shared_ptr<MShaderProgram> m_pShaderProgram = nullptr;

	MEMaterialType m_eMaterialType = MEMaterialType::EDefault;
	MECullMode m_eCullMode = MECullMode::ECullBack;
	bool m_bConservativeRasterizationEnable = false;
	Vector2i m_n2ShadingRate = {1, 1};
};

MORTY_SPACE_END