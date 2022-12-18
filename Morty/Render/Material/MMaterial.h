/**
 * @File         MMaterial
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#ifndef _M_MMATERIAL_H_
#define _M_MMATERIAL_H_
#include "Utility/MGlobal.h"
#include "Resource/MResource.h"
#include "Resource/MResource.h"
#include "Resource/MTextureResource.h"

#include "MShaderMacro.h"
#include "MShaderGroup.h"
#include "MShaderBuffer.h"
#include "Material/MShaderParamSet.h"

#include <vector>


enum class MERasterizerType
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
};

class MShader;
class MShaderResource;
class MORTY_API MMaterial : public MResource
{
public:
	MORTY_CLASS(MMaterial)
    MMaterial();
    virtual ~MMaterial();

	static MString GetResourceTypeName() { return "Material"; }
	static std::vector<MString> GetSuffixList() { return { "mat" }; }

public:

	MShader* GetVertexShader(){ return m_shaderGroup.GetVertexShader(); }
	MShader* GetPixelShader() { return m_shaderGroup.GetPixelShader(); }
	MShader* GetComputeShader() { return m_shaderGroup.GetComputeShader(); }

	std::vector<MShaderConstantParam*>* GetShaderParams();
	std::vector<MShaderSampleParam*>* GetSampleParams();
	std::vector<MShaderTextureParam*>* GetTextureParams();

	std::array<MShaderParamSet, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderParamSets() { return m_shaderGroup.GetShaderParamSets(); }
	const std::array<MShaderParamSet, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderParamSets() const { return m_shaderGroup.GetShaderParamSets(); }
	MShaderParamSet* GetMaterialParamSet() { return &m_shaderGroup.GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]; }
	MShaderParamSet* GetFrameParamSet() { return &m_shaderGroup.GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_FRAME]; }
	MShaderParamSet* GetMeshParamSet() { return &m_shaderGroup.GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_MESH]; }

	void SetTexutre(const MString& strName, std::shared_ptr<MResource> pTexResource);

	MShaderConstantParam* FindShaderParam(const MString& strName);

	void SetRasterizerType(const MERasterizerType& eType);
	MERasterizerType GetRasterizerType() const { return m_eRasterizerType; }

	void SetMaterialType(const MEMaterialType& eType);
	MEMaterialType GetMaterialType() const { return m_eMaterialType; }

	bool LoadVertexShader(std::shared_ptr<MResource> pResource);
	bool LoadPixelShader(std::shared_ptr<MResource> pResource);
	bool LoadComputeShader(std::shared_ptr<MResource> pResource);

	bool LoadVertexShader(const MString& strResource);
	bool LoadPixelShader(const MString& strResource);
	bool LoadComputeShader(const MString& strResource);

public:

	std::shared_ptr<MResource> GetVertexShaderResource() { return m_shaderGroup.GetVertexShaderResource(); }
	std::shared_ptr<MResource> GetPixelShaderResource() { return m_shaderGroup.GetPixelShaderResource(); }

	MShaderMacro& GetShaderMacro() { return m_shaderGroup.GetShaderMacro(); }
	const MShaderGroup& GetShaderGroup() const { return m_shaderGroup; }
	
	void SetMaterialID(const uint32_t& unID) { m_unMaterialID = unID; }
	uint32_t GetMaterialID() { return m_unMaterialID; }

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void Unload();

	virtual void CopyFrom(std::shared_ptr<const MResource> pResource) override;

	virtual void Encode(MString& strCode) override;
	virtual void Decode(MString& strCode) override;

	virtual bool SaveTo(const MString& strResourcePath) override;

protected:

	virtual bool Load(const MString& strResourcePath) override;

private:

	MShaderGroup m_shaderGroup;

	MERasterizerType m_eRasterizerType;
	MEMaterialType m_eMaterialType;

	uint32_t m_unMaterialID;
};

#endif
