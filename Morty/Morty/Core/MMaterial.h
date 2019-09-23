/**
 * @File         MMaterial
 * 
 * @Created      2019-08-27 19:22:28
 *
 * @Author       Morty
**/

#ifndef _M_MMATERIAL_H_
#define _M_MMATERIAL_H_
#include "MGlobal.h"
#include "MResource.h"
#include "MVertex.h"
#include "MObject.h"

#include <vector>

class MShader;
class MResource;
class MShaderResource;
class MMaterialResource;
class MORTY_CLASS MMaterial : public MObject
{
public:
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

	void SetPixelParam(const MString& strName, const MVariable& variable);

	bool Load(MResource* pResource);

	void Unload();

protected:

	void CleanTextureParams();

private:

	std::vector<MShaderParam> m_vVertexShaderParams;
	std::vector<MShaderParam> m_vPixelShaderParams;

	std::vector<MShaderTextureParam> m_vPixelTextureParams;
	std::vector<MResourceHolder*> m_vPixelTextureResHolder;

	MResourceHolder* m_pMaterialResource;
	MShader* m_pVertexShader;
	MShader* m_pPxielShader;


};


#endif
