/**
 * @File         MShader
 * 
 * @Created      2019-08-26 21:24:51
 *
 * @Author       Morty
**/

#ifndef _M_MSHADER_H_
#define _M_MSHADER_H_
#include "MGlobal.h"
#include "MString.h"

class MIRenderer;
class MShaderBuffer;
class MORTY_CLASS MShader
{
public:
	enum MEShaderType
	{
		Vertex = 1,
		Pixel = 2
	};
		
public:
    MShader();
    virtual ~MShader();

	void CompileShader(MIRenderer* pRenderer);
	MShaderBuffer* GetBuffer() { return m_pShaderBuffer; }
	const MEShaderType GetType() const { return m_eShaderType; }

private:

	friend class MShaderResource;
	MString m_strShaderPath;
	MEShaderType m_eShaderType;

	MShaderBuffer* m_pShaderBuffer;

};


#endif
