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
#include "MObject.h"

#include <vector>

class MShader;
class MShaderResource;
class MMaterialResource;
class MORTY_CLASS MMaterial : public MObject, public MRefCounter
{
public:
	M_OBJECT(MMaterial);
    MMaterial();
    virtual ~MMaterial();

	MShader* GetVertexShader(){ return m_pVertexShader; }
	MShader* GetPixelShader() { return m_pPxielShader; }

	std::vector<MShaderParam*>* GetShaderParams() { return &m_vShaderParams; }
	std::vector<MShaderTextureParam*>* GetTextureParams() { return &m_vTextureParams; }
	std::vector< MResourceHolder*>* GetTextures() { return &m_vTextureResHolder; }

	void SetTexutreParam(const MString& strName, MResource* pTexResource);

	void CompileShaderParams(const MEShaderParamType& eType);

	void SetParam(const MString& strName, const MVariant& variable);

	void SetRenderState(unsigned int& eType) { m_eRenderState = eType; }
	unsigned int GetRenderState() { return m_eRenderState; }

	bool Load(MMaterialResource* pResource);
	MResource* GetResource();

	void Unload();

protected:

	void RecompileShaderParams(std::vector<MShaderParam*>& vParams, std::vector<MShaderParam*>& vNewParams, const MEShaderParamType& eType);
	void RecompileShaderTextureParam(std::vector<MShaderTextureParam*>& vParams, std::vector<MResourceHolder*>& vResHolders, std::vector<MShaderTextureParam*>& vNewParams, const MEShaderParamType& eType);

	void CleanTextureParams();

private:

	//Shader Params
	std::vector<MShaderParam*> m_vShaderParams;

	//Texture
	std::vector<MShaderTextureParam*> m_vTextureParams;
	std::vector<MResourceHolder*> m_vTextureResHolder;

	//Material
	MResourceHolder* m_pMaterialResource;
	MShader* m_pVertexShader;
	MShader* m_pPxielShader;

	unsigned int m_eRenderState;

};

#endif
