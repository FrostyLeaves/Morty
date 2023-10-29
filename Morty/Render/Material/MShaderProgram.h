/**
 * @File         MShaderProgram
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include "Resource/MResource.h"
#include "Resource/MTextureResource.h"

#include "MShaderMacro.h"
#include "MShaderBuffer.h"
#include "Material/MShader.h"
#include "Material/MShaderPropertyBlock.h"

#include <vector>

struct MORTY_API MTextureResourceParam : public MShaderTextureParam
{
public:
	MTextureResourceParam();
	MTextureResourceParam(const MShaderTextureParam& param);

	void SetTexture(std::shared_ptr<MTexture> pTexture) override;
	std::shared_ptr<MTexture> GetTexture() override;

	void SetTexture(const std::shared_ptr<MTextureResource>& pTextureResource);
	std::shared_ptr<MTextureResource> GetTextureResource() const { return m_TextureRef.GetResource<MTextureResource>(); }

private:
	MResourceRef m_TextureRef;
};

class MShader;
class MShaderResource;
class MORTY_API MShaderProgram : public MTypeClass
{
public:
	enum class EUsage
	{
		EUnknow,
		EGraphics,
		ECompute,
	};

public:
	MORTY_CLASS(MShaderProgram)

public:
	explicit MShaderProgram() = default;
	explicit MShaderProgram(MEngine* pEngine, EUsage usage);
	~MShaderProgram() override;

public:
	static std::shared_ptr<MShaderProgram> MakeShared(MEngine* pEngine, EUsage usage);
	void InitializeShaderPropertyBlock();
public:

	bool LoadShader(std::shared_ptr<MResource> pResource);

	std::shared_ptr<MResource> GetShaderResource(MEShaderType eType) const { return m_shaders[size_t(eType)].resource.GetResource(); }
	MShader* GetShader(MEShaderType eType) const { return m_shaders[size_t(eType)].pShader; }

	void SetShaderMacro(const MShaderMacro& macro);
	MShaderMacro& GetShaderMacro() { return m_ShaderMacro; }

	const std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderPropertyBlocks() const { return m_vShaderSets; }
	std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderPropertyBlocks() { return m_vShaderSets; }

	EUsage GetUsage() const { return m_eUsage; }

public:

	std::shared_ptr<MShaderProgram> GetShared() const;

	void BindShaderBuffer(MShaderBuffer* pBuffer, const MEShaderType& eType);
	void UnbindShaderBuffer(const MEShaderType& eType);

	void ClearShader();
	
	static void CopyShaderParams(MEngine* pEngine, const std::shared_ptr<MShaderPropertyBlock>& target, const std::shared_ptr<const MShaderPropertyBlock>& source);

	std::shared_ptr<MShaderPropertyBlock> AllocShaderPropertyBlock(size_t nSetIdx);
	void ReleaseShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock);

protected:

	MEngine* GetEngine() const { return m_pEngine; }

    std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM> m_vShaderSets;
	std::set<std::shared_ptr<MShaderPropertyBlock>> m_tShaderPropertyBlockInstance;

	std::weak_ptr<MShaderProgram> m_pSelfPointer;


	struct MShaderDesc
	{
		MResourceRef resource = {};
		MShader* pShader = nullptr;
		int nShaderIdx = 0;
	};

	std::array<MShaderDesc, size_t(MEShaderType::TOTAL_NUM)> m_shaders;
	
	MShaderMacro m_ShaderMacro;
	MEngine* m_pEngine = nullptr;
	EUsage m_eUsage = EUsage::EUnknow;

};
