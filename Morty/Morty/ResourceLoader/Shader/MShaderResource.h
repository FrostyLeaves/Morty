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

#include <map>

#include "MResource.h"
#include "Shader/MShader.h"


class MORTY_API MShaderResource : public MResource
{
public:
	M_RESOURCE(MShaderResource)
    MShaderResource();
    virtual ~MShaderResource();

public:

    MShader* GetShaderByIndex(const int& nIndex);
    int FindShaderByMacroParam(const MShaderMacro& macro);

    MShader::MEShaderType GetShaderType() { return m_eShaderType; }

public:

	virtual void OnDelete() override;

protected:

	virtual bool Load(const MString& strResourcePath) override;

private:

    MShader::MEShaderType m_eShaderType;
    MString m_strShaderPath;

    std::vector<MShader*> m_vShaders;


private:

};


#endif
