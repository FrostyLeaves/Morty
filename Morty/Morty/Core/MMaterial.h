/**
 * @File         MMaterial
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       Pobrecito
**/

#ifndef _M_MMATERIAL_H_
#define _M_MMATERIAL_H_
#include "MGlobal.h"
#include "MRefCounter.h"
#include "MResource.h"
#include "MRenderStructure.h"
#include "MResource.h"
#include "MIRenderer.h"
#include "MShaderMacro.h"

#include <vector>

class MShader;
class MShaderResource;
class MORTY_CLASS MMaterial : public MResource
{
public:
	M_RESOURCE(MMaterial)
    MMaterial();
    virtual ~MMaterial();

	MShader* GetVertexShader(){ return m_pVertexShader; }
	MShader* GetPixelShader() { return m_pPixelShader; }

	std::vector<MShaderParam>* GetShaderParams() { return &m_vShaderParams; }
	std::vector<MShaderTextureParam>* GetTextureParams() { return &m_vTextureParams; }
	std::vector< MResourceKeeper>* GetTextures() { return &m_vTextureResKeeper; }

	void SetTexutreParam(const MString& strName, MResource* pTexResource);
	void SetTexutreParam(const unsigned int& unIndex, MResource* pTexResource);

	void SetRasterizerType(const MERasterizerType& eType) { m_eRasterizerType = eType; }
	MERasterizerType GetRasterizerType() const { return m_eRasterizerType; }

	void SetMaterialType(const MEMaterialType& eType) { m_eMaterialType = eType; }
	MEMaterialType GetMaterialType() const { return m_eMaterialType; }

	bool LoadVertexShader(MResource* pResource);
	bool LoadPixelShader(MResource* pResource);

	MResource* GetVertexShaderResource() { return m_VertexResource.GetResource(); }
	MResource* GetPixelShaderResource() { return m_VertexResource.GetResource(); }

	MShaderMacro* GetShaderMacro() { return &m_ShaderMacro; }
	
public:

	virtual void OnDelete() override;

	void Unload();

	void CompileShaderParams(const MEShaderParamType& eType);

	virtual void CopyFrom(const MResource* pResource) override;

	virtual void Encode(MString& strCode) override;
	virtual void Decode(MString& strCode) override;

	virtual bool SaveTo(const MString& strResourcePath) override;

protected:

	virtual bool Load(const MString& strResourcePath) override;

	void RecompileShaderParams(std::vector<MShaderParam>& vParams, std::vector<MShaderParam*>& vNewParams, const MEShaderParamType& eType);
	void RecompileShaderTextureParam(std::vector<MShaderTextureParam>& vParams, std::vector<MResourceKeeper>& vResHolders, std::vector<MShaderTextureParam*>& vNewParams, const MEShaderParamType& eType);

	void CleanTextureParams();
	void CleanShaderParams();

	void UpdateShaderMacro();

private:

	//Shader Params
	std::vector<MShaderParam> m_vShaderParams;

	//Texture
	std::vector<MShaderTextureParam> m_vTextureParams;
	std::vector<MResourceKeeper> m_vTextureResKeeper;

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
};

#endif
