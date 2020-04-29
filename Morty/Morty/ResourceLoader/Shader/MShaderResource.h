/**
 * @File         MShaderResource
 * 
 * @Created      2019-08-26 20:26:13
 *
 * @Author       Pobrecito
**/

#ifndef _M_MSHADERRESOURCE_H_
#define _M_MSHADERRESOURCE_H_
#include "MGlobal.h"

#include "MResource.h"

class MShader;
class MORTY_CLASS MShaderResource : public MResource
{
public:
    MShaderResource();
    virtual ~MShaderResource();

public:

	MShader* GetShaderTemplate() { return m_pShaderTemplate; }

public:

	virtual void OnDelete() override;

protected:

	virtual bool Load(const MString& strResourcePath) override;

private:

	MShader* m_pShaderTemplate;

private:

};


#endif
