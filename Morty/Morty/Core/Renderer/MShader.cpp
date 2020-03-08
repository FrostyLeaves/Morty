#include "MShader.h"
#include "MIDevice.h"
#include "MVertex.h"
#include "MRenderStructure.h"

#include <vector>

MShader::MShader()
	: m_pShaderBuffer(nullptr)
	, m_eShaderType(None)
{
}

bool MShader::CompileShader(MIDevice* pDevice)
{
	if (false == pDevice->CompileShader(&m_pShaderBuffer, m_strShaderPath, m_eShaderType))
		return false;

	return true;
}

void MShader::CleanShader(MIDevice* pDevice)
{
	pDevice->CleanShader(&m_pShaderBuffer);
}

MShader::~MShader()
{

}
