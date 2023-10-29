#include "MShaderBuffer.h"


MShaderBuffer::MShaderBuffer()
{
    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_MATERIAL] = MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_MATERIAL);
    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_FRAME] = MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_FRAME);
    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_MESH] = MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_MESH);
    m_vShaderSets[MRenderGlobal::SHADER_PARAM_SET_SKELETON] = MShaderPropertyBlock::MakeShared(nullptr, MRenderGlobal::SHADER_PARAM_SET_SKELETON);
}
