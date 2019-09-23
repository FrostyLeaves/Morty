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

class MShader;
class MShaderResource;
class MORTY_CLASS MMaterialResource : public MResource
{
public:
    MMaterialResource();
    virtual ~MMaterialResource();

	MShader* GetVertexShader() { return m_pVertexShader; }
	MShader* GetPixelShader() { return m_pPixelShader; }

	bool LoadVertexShader(MResource* pResource);
	bool LoadPixelShader(MResource* pResource);

protected:

	virtual bool Load(const MString& strResourcePath) override;

private:


	MShader* m_pVertexShader;
	MShader* m_pPixelShader;

	MResourceHolder* m_pVertexResource;
	MResourceHolder* m_pPixelResource;

};


#endif
