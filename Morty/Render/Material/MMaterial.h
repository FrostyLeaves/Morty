/**
 * @File         MMaterial
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       DoubleYe
**/

#ifndef _M_MMATERIAL_H_
#define _M_MMATERIAL_H_
#include "MGlobal.h"
#include "MResource.h"
#include "MResource.h"
#include "MRefCounter.h"

#include "MShaderMacro.h"
#include "MShaderBuffer.h"
#include "MShaderParamSet.h"

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

struct MORTY_API MShaderRefTextureParam : public MShaderTextureParam
{
public:
	MShaderRefTextureParam();
	MShaderRefTextureParam(const MShaderTextureParam& param);

	MResourceKeeper m_TextureRef;
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

	MShader* GetVertexShader(){ return m_pVertexShader; }
	MShader* GetPixelShader() { return m_pPixelShader; }

	std::vector<MShaderConstantParam*>* GetShaderParams();
	std::vector<MShaderSampleParam*>* GetSampleParams();
	std::vector<MShaderTextureParam*>* GetTextureParams();

	MShaderParamSet* GetShaderParamSets() { return m_vShaderSets; }
	MShaderParamSet* GetMaterialParamSet() { return &m_vShaderSets[MGlobal::SHADER_PARAM_SET_MATERIAL]; }
	MShaderParamSet* GetFrameParamSet() { return &m_vShaderSets[MGlobal::SHADER_PARAM_SET_FRAME]; }
	MShaderParamSet* GetMeshParamSet() { return &m_vShaderSets[MGlobal::SHADER_PARAM_SET_MESH]; }

	void SetTexutreParam(const MString& strName, MResource* pTexResource);
	void SetTexutreParam(const uint32_t& unIndex, MResource* pTexResource);

	MShaderConstantParam* FindShaderParam(const MString& strName);

	void SetRasterizerType(const MERasterizerType& eType);
	MERasterizerType GetRasterizerType() const { return m_eRasterizerType; }

	void SetMaterialType(const MEMaterialType& eType);
	MEMaterialType GetMaterialType() const { return m_eMaterialType; }

	bool LoadVertexShader(MResource* pResource);
	bool LoadPixelShader(MResource* pResource);

	bool LoadVertexShader(const MString& strResource);
	bool LoadPixelShader(const MString& strResource);

public:

	MResource* GetVertexShaderResource() { return m_VertexResource.GetResource(); }
	MResource* GetPixelShaderResource() { return m_PixelResource.GetResource(); }

	MShaderMacro* GetShaderMacro() { return &m_ShaderMacro; }
	
	void SetMaterialID(const uint32_t& unID) { m_unMaterialID = unID; }
	uint32_t GetMaterialID() { return m_unMaterialID; }

public:

	virtual void OnCreated() override;
	virtual void OnDelete() override;

	void Unload();

	virtual void CopyFrom(const MResource* pResource) override;

	virtual void Encode(MString& strCode) override;
	virtual void Decode(MString& strCode) override;

	virtual bool SaveTo(const MString& strResourcePath) override;

protected:

	virtual bool Load(const MString& strResourcePath) override;

	void CopyShaderParamSet(MShaderParamSet& target, const MShaderParamSet& source);

	void BindShaderBuffer(MShaderBuffer* pBuffer, const MEShaderParamType& eType);
	void UnbindShaderBuffer(const MEShaderParamType& eType);

	void ClearParams();

private:

    MShaderParamSet m_vShaderSets[MGlobal::SHADER_PARAM_SET_NUM];
	


	
private:

	//Material
	MResourceKeeper m_VertexResource;
	MResourceKeeper m_PixelResource;

	MShader* m_pVertexShader;
	MShader* m_pPixelShader;

	MERasterizerType m_eRasterizerType;
	MEMaterialType m_eMaterialType;

	int m_nVertexShaderIndex;
	int m_nPixelShaderIndex;
	MShaderMacro m_ShaderMacro;

	uint32_t m_unMaterialID;
};

#endif
