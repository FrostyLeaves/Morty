#include "MShaderBuffer.h"
#include "MShaderParameterSet.h"

using namespace morty;

MShaderBuffer::MShaderBuffer()
{
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_MATERIAL] =
            std::make_shared<MShaderParameterSet>(nullptr, MRenderGlobal::SHADER_PARAM_SET_MATERIAL);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_FRAME] =
            std::make_shared<MShaderParameterSet>(nullptr, MRenderGlobal::SHADER_PARAM_SET_FRAME);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_MESH] =
            std::make_shared<MShaderParameterSet>(nullptr, MRenderGlobal::SHADER_PARAM_SET_MESH);
    m_shaderSets[MRenderGlobal::SHADER_PARAM_SET_OTHER] =
            std::make_shared<MShaderParameterSet>(nullptr, MRenderGlobal::SHADER_PARAM_SET_OTHER);
}
