/**
 * @File         MMaterialResource
 * 
 * @Created      2019-09-01 15:25:21
 *
 * @Author       Morty
**/

#ifndef _M_MMATERIALRESOURCE_H_
#define _M_MMATERIALRESOURCE_H_
#include "MGlobal.h"
#include "MResource.h"

class MShaderResource;
class MORTY_CLASS MMaterialResource : public MResource
{
public:
    MMaterialResource();
    virtual ~MMaterialResource();

	MShaderResource* GetVertexShaderResource() { return m_pVertexShader; }
	MShaderResource* GetPixelShaderResource() { return m_pPixelShader; }

	bool LoadVertexShader(MResource* pResource);
	bool LoadPixelShader(MResource* pResource);


protected:

	virtual bool Load(const MString& strResourcePath) override;

private:


	MShaderResource* m_pVertexShader;
	MShaderResource* m_pPixelShader;

};


#endif
