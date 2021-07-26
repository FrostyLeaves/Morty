#include "MShader.h"
#include "MIDevice.h"
#include "MVertex.h"

#include <vector>

MShader::MShader()
	: m_pShaderBuffer(nullptr)
	, m_eShaderType(MEShaderType::ENone)
{
}

bool MShader::CompileShader(MIDevice* pDevice)
{
	if (false == pDevice->CompileShader(this))
		return false;

	return true;
}

void MShader::CleanShader(MIDevice* pDevice)
{
	pDevice->CleanShader(this);
}

MShader::~MShader()
{

}
