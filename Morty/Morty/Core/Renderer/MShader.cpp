#include "MShader.h"
#include "MIDevice.h"
#include "MVertex.h"

#include <vector>

MShader::MShader()
{
	m_pShaderBuffer = nullptr;
}

void MShader::CompileShader(MIDevice* pDevice)
{
	pDevice->CompileShader(&m_pShaderBuffer, m_strShaderPath, m_eShaderType);
}

void MShader::CleanShader(MIDevice* pDevice)
{
	pDevice->CleanShader(&m_pShaderBuffer);
}

MShader::~MShader()
{

}
