/**
 * @File         MShaderGroup
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSHADER_GROUP_H_
#define _M_MSHADER_GROUP_H_
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
class MORTY_API MShaderGroup : public MTypeClass
{
public:
	MORTY_CLASS(MShaderGroup)
	MShaderGroup();
    virtual ~MShaderGroup();

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

	const std::array<MShaderParamSet, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderParamSets() const { return m_vShaderSets; }
	std::array<MShaderParamSet, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderParamSets() { return m_vShaderSets; }

public:

	void CopyShaderParamSet(MShaderParamSet& target, const MShaderParamSet& source);

	void BindShaderBuffer(MShaderBuffer* pBuffer, const MEShaderParamType& eType);
	void UnbindShaderBuffer(MEngine* pEngine, const MEShaderParamType& eType);

	void ClearShader(MEngine* pEngine);
	
	void CopyShaderParams(MEngine* pEngine, MShaderParamSet& target, const MShaderParamSet& source);

protected:

    std::array<MShaderParamSet, MRenderGlobal::SHADER_PARAM_SET_NUM> m_vShaderSets;
	
	//Material
	MResourceKeeper m_VertexResource;
	MResourceKeeper m_PixelResource;
	MResourceKeeper m_ComputeResource;

	MShader* m_pVertexShader;
	MShader* m_pPixelShader;
	MShader* m_pComputeShader;

	int m_nVertexShaderIndex;
	int m_nPixelShaderIndex;
	int m_nComputeShaderIndex;

	MShaderMacro m_ShaderMacro;
};

#endif
