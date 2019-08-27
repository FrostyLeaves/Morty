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

class MShader;
class MResource;
class MShaderResource;
class MORTY_CLASS MMaterial
{
public:
    MMaterial();
    virtual ~MMaterial();

	bool LoadVertexShader(MResource* pResource);
	bool LoadPixelShader(MResource* pResource);

	MShader* GetVertexShader();
	MShader* GetPixelShader();

private:

	MShaderResource* m_pVertexShader;
	MShaderResource* m_pPixelShader;

	MMaterial* m_pNextPass;

};


#endif
