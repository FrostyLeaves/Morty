#include "MShader.h"
#include "MIRenderer.h"

MShader::MShader()
{
	m_pShaderBuffer = nullptr;
}

void MShader::CompileShader(MIRenderer* pRenderer)
{
	pRenderer->CompileShader(&m_pShaderBuffer, m_strShaderPath, m_eShaderType);
}

MShader::~MShader()
{

}
