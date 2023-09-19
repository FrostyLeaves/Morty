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

class MORTY_API MaterialKey
{
public:
	static const MString Albedo;
	static const MString Normal;
	static const MString Metallic;
	static const MString Roughness;
	static const MString AmbientOcc;
	static const MString Height;
	static const MString Emission;
};

class MShader;
class MShaderResource;
class MORTY_API MMaterial : public MResource
{
public:
	MORTY_CLASS(MMaterial)
    MMaterial();
    ~MMaterial() override = default;
	
	MShader* GetVertexShader() const { return m_pShaderProgram->GetVertexShader(); }
	MShader* GetPixelShader() const { return m_pShaderProgram->GetPixelShader(); }

	std::vector<std::shared_ptr<MShaderConstantParam>>& GetShaderParams();
	std::vector<std::shared_ptr<MShaderSampleParam>>& GetSampleParams();
	std::vector<std::shared_ptr<MShaderTextureParam>>& GetTextureParams();

	std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderPropertyBlocks() { return m_pShaderProgram->GetShaderPropertyBlocks(); }
	const std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderPropertyBlocks() const { return m_pShaderProgram->GetShaderPropertyBlocks(); }
	std::shared_ptr<MShaderPropertyBlock> GetMaterialPropertyBlock() const { return m_pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]; }
	std::shared_ptr<MShaderPropertyBlock> GetFramePropertyBlock() const { return m_pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_FRAME]; }
	std::shared_ptr<MShaderPropertyBlock> GetMeshPropertyBlock() const { return m_pShaderProgram->GetShaderPropertyBlocks()[MRenderGlobal::SHADER_PARAM_SET_MESH]; }

	void SetTexture(const MString& strName, std::shared_ptr<MResource> pTexResource);

	std::shared_ptr<MShaderConstantParam> FindShaderParam(const MString& strName);
	std::shared_ptr<MShaderSampleParam> FindSample(const MString& strName);
	std::shared_ptr<MShaderTextureParam> FindTexture(const MString& strName);

	void SetCullMode(const MECullMode& eType);
	MECullMode GetCullMode() const { return m_eCullMode; }

	void SetMaterialType(const MEMaterialType& eType);
	MEMaterialType GetMaterialType() const { return m_eMaterialType; }

	bool LoadVertexShader(std::shared_ptr<MResource> pResource);
	bool LoadPixelShader(std::shared_ptr<MResource> pResource);

	bool LoadVertexShader(const MString& strResource);
	bool LoadPixelShader(const MString& strResource);

	void SetBatchInstanceEnable(const bool& bBatch) { m_bBatchInstanceEnable = bBatch; }
	bool GetBatchInstanceEnable() const { return m_bBatchInstanceEnable; }

public:

	std::shared_ptr<MResource> GetVertexShaderResource() const { return m_pShaderProgram->GetVertexShaderResource(); }
	std::shared_ptr<MResource> GetPixelShaderResource() const { return m_pShaderProgram->GetPixelShaderResource(); }

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

	std::shared_ptr<MShaderProgram> m_pShaderProgram;

	MEMaterialType m_eMaterialType = MEMaterialType::EDefault;
	MECullMode m_eCullMode = MECullMode::ECullBack;

	bool m_bBatchInstanceEnable = false;
};
