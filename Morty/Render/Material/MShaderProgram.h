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
#include "Resource/MResource.h"
#include "Resource/MTextureResource.h"

#include "MShaderMacro.h"
#include "MShaderBuffer.h"
#include "Material/MShaderPropertyBlock.h"

#include <vector>

struct MORTY_API MTextureResourceParam : public MShaderTextureParam
{
public:
	MTextureResourceParam();
	MTextureResourceParam(const MShaderTextureParam& param);

	virtual void SetTexture(std::shared_ptr<MTexture> pTexture) override;
	virtual std::shared_ptr<MTexture> GetTexture() override;

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
	explicit MShaderProgram(EUsage usage);

public:
    virtual ~MShaderProgram();
	static std::shared_ptr<MShaderProgram> MakeShared(EUsage usage);
	void InitializeShaderPropertyBlock();
public:

	bool LoadVertexShader(MEngine* pEngine, std::shared_ptr<MResource> pResource);
	bool LoadPixelShader(MEngine* pEngine, std::shared_ptr<MResource> pResource);
	bool LoadComputeShader(MEngine* pEngine, std::shared_ptr<MResource> pResource);

	std::shared_ptr<MResource> GetVertexShaderResource() const { return m_VertexResource.GetResource(); }
	std::shared_ptr<MResource> GetPixelShaderResource() const { return m_PixelResource.GetResource(); }
	std::shared_ptr<MResource> GetComputeShaderResource() const { return m_ComputeResource.GetResource(); }

	MShader* GetVertexShader(){ return m_pVertexShader; }
	MShader* GetPixelShader() { return m_pPixelShader; }
	MShader* GetComputeShader() { return m_pComputeShader; }

	void SetShaderMacro(const MShaderMacro& macro) { m_ShaderMacro = macro; }
	MShaderMacro& GetShaderMacro() { return m_ShaderMacro; }

	std::vector<MShaderConstantParam*>* GetShaderParams();
	std::vector<MShaderSampleParam*>* GetSampleParams();
	std::vector<MShaderTextureParam*>* GetTextureParams();

	const std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderPropertyBlocks() const { return m_vShaderSets; }
	std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderPropertyBlocks() { return m_vShaderSets; }

	EUsage GetUsage() const { return m_eUsage; }

	void GenerateProgram(MIDevice* pDevice);
	void DestroyProgram(MIDevice* pDevice);

public:

	std::shared_ptr<MShaderProgram> GetShared() const;

	void BindShaderBuffer(MShaderBuffer* pBuffer, const MEShaderParamType& eType);
	void UnbindShaderBuffer(MEngine* pEngine, const MEShaderParamType& eType);

	void ClearShader(MEngine* pEngine);
	
	static void CopyShaderParams(MEngine* pEngine, const std::shared_ptr<MShaderPropertyBlock>& target, const std::shared_ptr<const MShaderPropertyBlock>& source);

	std::shared_ptr<MShaderPropertyBlock> AllocShaderPropertyBlock(size_t nSetIdx);
	void ReleaseShaderPropertyBlock(const std::shared_ptr<MShaderPropertyBlock>& pShaderPropertyBlock);

protected:

    std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM> m_vShaderSets;
	std::set<std::shared_ptr<MShaderPropertyBlock>> m_tShaderPropertyBlockInstance;

	std::weak_ptr<MShaderProgram> m_pSelfPointer;
	
	MResourceRef m_VertexResource = MResourceRef();
	MResourceRef m_PixelResource = MResourceRef();
	MResourceRef m_ComputeResource = MResourceRef();

	MShader* m_pVertexShader = nullptr;
	MShader* m_pPixelShader = nullptr;
	MShader* m_pComputeShader = nullptr;

	int m_nVertexShaderIndex = 0;
	int m_nPixelShaderIndex = 0;
	int m_nComputeShaderIndex = 0;

	MShaderMacro m_ShaderMacro;
	EUsage m_eUsage = EUsage::EUnknow;

};
