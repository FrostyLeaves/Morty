#include "MShaderBuffer.h"

using namespace morty;

MShaderBuffer::MShaderBuffer()
{
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_MATERIAL] =
            MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_MATERIAL);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_FRAME] =
            MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_FRAME);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_MESH] =
            MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_MESH);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_OTHER] =
            MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_OTHER);
}
