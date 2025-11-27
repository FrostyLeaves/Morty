#include "MShader.h"
#include "Mesh/MVertex.h"
#include "RHI/Abstract/MIDevice.h"

using namespace morty;

bool MShader::CompileShader(MIDevice* pDevice)
{
    if (!pDevice->CompileShader(this)) return false;

    m_compiled = true;
    return true;
}

void MShader::CleanShader(MIDevice* pDevice)
{
    pDevice->CleanShader(this);
    m_shaderBuffer = nullptr;
    m_compiled     = false;
}

void MShader::SetBuffer(MShaderBuffer* shaderBuffer)
{
    MORTY_ASSERT(m_shaderBuffer == nullptr);
    m_shaderBuffer = shaderBuffer;
}

void MShader::SetShaderPropertyBlock(const MShaderPropertyBlock& propertyBlock) { m_propertyBlock = propertyBlock; }

MShaderBuffer* MShader::GetBuffer() const { return m_shaderBuffer; }
