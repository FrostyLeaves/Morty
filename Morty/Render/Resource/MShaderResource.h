/**
 * @File         MShaderResource
 * 
 * @Created      2019-08-26 20:26:13
 *
 * @Author       DoubleYe
**/

#ifndef _M_MSHADERRESOURCE_H_
#define _M_MSHADERRESOURCE_H_
#include "Utility/MGlobal.h"

#include <map>

#include "Resource/MResource.h"
#include "Material/MShader.h"


class MORTY_API MShaderResource : public MResource
{
public:
	MORTY_CLASS(MShaderResource)
    MShaderResource();
    virtual ~MShaderResource();

	static MString GetResourceTypeName() { return "Shader"; }
	static std::vector<MString> GetSuffixList() { return { "mvs", "mps", "mcs" }; }

public:

    MShader* GetShaderByIndex(const int& nIndex);
    int FindShaderByMacroParam(const MShaderMacro& macro);

	MEShaderType GetShaderType() { return m_eShaderType; }

public:

	virtual void OnDelete() override;

protected:

	virtual bool Load(const MString& strResourcePath) override;

private:

    MEShaderType m_eShaderType;
    MString m_strShaderPath;

    std::vector<MShader*> m_vShaders;


private:

};


#endif
