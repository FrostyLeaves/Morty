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
class MResource;
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

	std::vector<MShaderParam>& GetVertexShaderParams() { return m_vVertexShaderParams; }
	std::vector<MShaderParam>& GetPixelShaderParams() { return m_vPixelShaderParams; }
	std::vector<MShaderTextureParam>& GetPixelTextureParams(){ return m_vPixelTextureParams; }

	void SetPixelTexutreParam(const MString& strName, MResource* pTexResource);

	void CompileVertexShaderParams();
	void CompilePixelShaderParams();

	void SetPixelParam(const MString& strName, const MVariant& variable);

	void SetRenderState(unsigned int& eType) { m_eRenderState = eType; }
	unsigned int GetRenderState() { return m_eRenderState; }

	bool Load(MResource* pResource);

	void Unload();

	void CleanTextureParams();


private:

	//Shader Params
	std::vector<MShaderParam> m_vVertexShaderParams;
	std::vector<MShaderParam> m_vPixelShaderParams;

	//Texture
	std::vector<MShaderTextureParam> m_vPixelTextureParams;
	std::vector<MResourceHolder*> m_vPixelTextureResHolder;

	//Material
	MResourceHolder* m_pMaterialResource;
	MShader* m_pVertexShader;
	MShader* m_pPxielShader;

	unsigned int m_eRenderState;
};


#endif
