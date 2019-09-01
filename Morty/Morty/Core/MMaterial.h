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

	MShader* GetVertexShader();
	MShader* GetPixelShader();

	std::vector<MShaderParam>& GetVertexShaderParams() { return m_vVertexShaderParams; }
	std::vector<MShaderParam>& GetPixelShaderParams() { return m_vPixelShaderParams; }

	void CompileVertexShaderParams();
	void CompilePixelShaderParams();


	bool Load(MResource* pResource);

private:

	std::vector<MShaderParam> m_vVertexShaderParams;
	std::vector<MShaderParam> m_vPixelShaderParams;


	MMaterialResource* m_pResource;

	MMaterial* m_pNextPass;

};


#endif
