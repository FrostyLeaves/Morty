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

#include <vector>

class MShader;
class MShaderResource;
class MORTY_CLASS MMaterial : public MResource
{
public:
    MMaterial();
    virtual ~MMaterial();

	MShader* GetVertexShader(){ return m_pVertexShader; }
	MShader* GetPixelShader() { return m_pPixelShader; }

	std::vector<MShaderParam>* GetShaderParams() { return &m_vShaderParams; }
	std::vector<MShaderTextureParam>* GetTextureParams() { return &m_vTextureParams; }
	std::vector< MResourceHolder*>* GetTextures() { return &m_vTextureResHolder; }

	void SetTexutreParam(const MString& strName, MResource* pTexResource);

	void SetRenderState(unsigned int& eType) { m_eRenderState = eType; }
	unsigned int GetRenderState() { return m_eRenderState; }

	bool LoadVertexShader(MResource* pResource);
	bool LoadPixelShader(MResource* pResource);

	MResource* GetVertexShaderResource() { return m_pVertexResource ? m_pVertexResource->GetResource() : nullptr; }
	MResource* GetPixelShaderResource() { return m_pPixelResource ? m_pPixelResource->GetResource() : nullptr; }

	
public:

	void Unload();

	void CompileShaderParams(const MEShaderParamType& eType);

	const MMaterial& operator= (const MMaterial& mat);

	virtual void Encode(MString& strCode) override;
	virtual void Decode(MString& strCode) override;

protected:

	virtual bool Load(const MString& strResourcePath) override;

	void RecompileShaderParams(std::vector<MShaderParam>& vParams, std::vector<MShaderParam*>& vNewParams, const MEShaderParamType& eType);
	void RecompileShaderTextureParam(std::vector<MShaderTextureParam>& vParams, std::vector<MResourceHolder*>& vResHolders, std::vector<MShaderTextureParam*>& vNewParams, const MEShaderParamType& eType);

	void CleanTextureParams();

private:

	//Shader Params
	std::vector<MShaderParam> m_vShaderParams;

	//Texture
	std::vector<MShaderTextureParam> m_vTextureParams;
	std::vector<MResourceHolder*> m_vTextureResHolder;

	//Material
	MResourceHolder* m_pVertexResource;
	MResourceHolder* m_pPixelResource;

	MShader* m_pVertexShader;
	MShader* m_pPixelShader;

	unsigned int m_eRenderState;
};

#endif
