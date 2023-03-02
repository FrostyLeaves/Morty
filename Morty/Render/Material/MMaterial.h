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
#include "MShaderProgram.h"
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
	static const MString Emission;
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

	MShader* GetVertexShader(){ return m_pShaderProgram->GetVertexShader(); }
	MShader* GetPixelShader() { return m_pShaderProgram->GetPixelShader(); }
	MShader* GetComputeShader() { return m_pShaderProgram->GetComputeShader(); }

	std::vector<std::shared_ptr<MShaderConstantParam>>& GetShaderParams();
	std::vector<std::shared_ptr<MShaderSampleParam>>& GetSampleParams();
	std::vector<std::shared_ptr<MShaderTextureParam>>& GetTextureParams();

	std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderParamSets() { return m_pShaderProgram->GetShaderParamSets(); }
	const std::array<std::shared_ptr<MShaderPropertyBlock>, MRenderGlobal::SHADER_PARAM_SET_NUM>& GetShaderParamSets() const { return m_pShaderProgram->GetShaderParamSets(); }
	std::shared_ptr<MShaderPropertyBlock> GetMaterialParamSet() const { return m_pShaderProgram->GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_MATERIAL]; }
	std::shared_ptr<MShaderPropertyBlock> GetFrameParamSet() const { return m_pShaderProgram->GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_FRAME]; }
	std::shared_ptr<MShaderPropertyBlock> GetMeshParamSet() const { return m_pShaderProgram->GetShaderParamSets()[MRenderGlobal::SHADER_PARAM_SET_MESH]; }

	void SetTexture(const MString& strName, std::shared_ptr<MResource> pTexResource);

	std::shared_ptr<MShaderConstantParam> FindShaderParam(const MString& strName);
	std::shared_ptr<MShaderSampleParam> FindSample(const MString& strName);
	std::shared_ptr<MShaderTextureParam> FindTexture(const MString& strName);

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

	std::shared_ptr<MResource> GetVertexShaderResource() const { return m_pShaderProgram->GetVertexShaderResource(); }
	std::shared_ptr<MResource> GetPixelShaderResource() const { return m_pShaderProgram->GetPixelShaderResource(); }

	MShaderMacro& GetShaderMacro() const { return m_pShaderProgram->GetShaderMacro(); }
	const std::shared_ptr<MShaderProgram>& GetShaderProgram() const { return m_pShaderProgram; }
	
	void SetMaterialID(const uint32_t& unID) { m_unMaterialID = unID; }
	uint32_t GetMaterialID() { return m_unMaterialID; }

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void Unload();

	virtual void CopyFrom(std::shared_ptr<const MResource> pResource) override;

	virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb) const;
	virtual void Deserialize(const void* pBufferPointer);

	virtual bool SaveTo(const MString& strResourcePath) override;

protected:

	virtual bool Load(const MString& strResourcePath) override;

private:

	std::shared_ptr<MShaderProgram> m_pShaderProgram;

	MERasterizerType m_eRasterizerType;
	MEMaterialType m_eMaterialType;

	uint32_t m_unMaterialID;
};

#endif
