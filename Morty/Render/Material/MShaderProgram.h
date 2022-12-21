/**
 * @File         MShaderProgram
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSHADER_PROGRAM_H_
#define _M_MSHADER_PROGRAM_H_
#include "Utility/MGlobal.h"
#include "Resource/MResource.h"
#include "Resource/MResource.h"
#include "Resource/MTextureResource.h"

#include "MShaderMacro.h"
#include "MShaderBuffer.h"
#include "Material/MShaderParamSet.h"

#include <vector>

struct MORTY_API MTextureResourceParam : public MShaderTextureParam
{
public:
	MTextureResourceParam();
	MTextureResourceParam(const MShaderTextureParam& param);

	virtual void SetTexture(MTexture* pTexture) override;
	virtual MTexture* GetTexture() override;

	void SetTexture(const std::shared_ptr<MTextureResource>& pTextureResource);
	std::shared_ptr<MTextureResource> GetTextureResource() { return m_TextureRef.GetResource<MTextureResource>(); }

private:
	MResourceKeeper m_TextureRef;
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

protected:
	explicit MShaderProgram() = default;
	explicit MShaderProgram(EUsage usage);

public:
    virtual ~MShaderProgram();
	static std::shared_ptr<MShaderProgram> MakeShared(EUsage usage);

public:

	bool LoadVertexShader(MEngine* pEngine, std::shared_ptr<MResource> pResource);
	bool LoadPixelShader(MEngine* pEngine, std::shared_ptr<MResource> pResource);
	bool LoadComputeShader(MEngine* pEngine, std::shared_ptr<MResource> pResource);

	std::shared_ptr<MResource> GetVertexShaderResource() { return m_VertexResource.GetResource(); }
	std::shared_ptr<MResource> GetPixelShaderResource() { return m_PixelResource.GetResource(); }
	std::shared_ptr<MResource> GetComputeShaderResource() { return m_ComputeResource.GetResource(); }

	MShader* GetVertexShader(){ return m_pVertexShader; }
	MShader* GetPixelShader() { return m_pPixelShader; }
	MShader* GetComputeShader() { return m_pComputeShader; }
	MShaderMacro& GetShaderMacro() { return m_ShaderMacro; }

	std::vector<MShaderConstantParam*>* GetShaderParams();
	std::vector<MShaderSampleParam*>* GetSampleParams();
	std::vector<MShaderTextureParam*>* GetTextureParams();

	const std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderParamSets() const { return m_vShaderSets; }
	std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderParamSets() { return m_vShaderSets; }

	EUsage GetUsage() const { return m_eUsage; }

	void GenerateProgram(MIDevice* pDevice);
	void DestroyProgram(MIDevice* pDevice);

public:

	std::shared_ptr<MShaderProgram> GetShared() const;

	void BindShaderBuffer(MShaderBuffer* pBuffer, const MEShaderParamType& eType);
	void UnbindShaderBuffer(MEngine* pEngine, const MEShaderParamType& eType);

	void ClearShader(MEngine* pEngine);
	
	static void CopyShaderParams(MEngine* pEngine, const std::shared_ptr<MShaderPropertyBlock>& target, const std::shared_ptr<const MShaderPropertyBlock>& source);

	std::shared_ptr<MShaderPropertyBlock> AllocShaderParamSet(size_t nSetIdx);
	void ReleaseShaderParamSet(const std::shared_ptr<MShaderPropertyBlock>& pShaderParamSet);

protected:

    std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM> m_vShaderSets;
	std::set<std::shared_ptr<MShaderPropertyBlock>> m_tShaderParamSetInstance;

	std::weak_ptr<MShaderProgram> m_pSelfPointer;
	
	MResourceKeeper m_VertexResource = nullptr;
	MResourceKeeper m_PixelResource = nullptr;
	MResourceKeeper m_ComputeResource = nullptr;

	MShader* m_pVertexShader = nullptr;
	MShader* m_pPixelShader = nullptr;
	MShader* m_pComputeShader = nullptr;

	int m_nVertexShaderIndex = 0;
	int m_nPixelShaderIndex = 0;
	int m_nComputeShaderIndex = 0;

	MShaderMacro m_ShaderMacro;
	EUsage m_eUsage = EUsage::EUnknow;


public:

#if RENDER_GRAPHICS == MORTY_VULKAN
	VkPipelineLayout m_vkPipelineLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayout> m_vDescriptorSetLayouts = {};
#endif
};

#endif