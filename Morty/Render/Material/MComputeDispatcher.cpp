#include "MComputeDispatcher.h"
#include "Engine/MEngine.h"
#include "RHI/Abstract/MIDevice.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MShaderResource.h"
#include "Shader/MShader.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MComputeDispatcher, MObject)

bool MComputeDispatcher::LoadComputeShader(std::shared_ptr<MResource> pResource)
{
    bool bResult = m_shaderProgram->LoadShader(pResource);

    return bResult;
}

bool MComputeDispatcher::LoadComputeShader(const MString& strResource)
{
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResource))
        return LoadComputeShader(pResource);

    return false;
}

MShader* MComputeDispatcher::GetComputeShader() { return m_shaderProgram->GetShader(MEShaderType::ECompute); }

void     MComputeDispatcher::OnCreated()
{
    Super::OnCreated();

    m_shaderProgram = MShaderProgram::MakeShared(GetEngine(), MShaderProgram::EUsage::ECompute);
}

void MComputeDispatcher::OnDelete()
{
    m_shaderProgram->ClearShader();

    Super::OnDelete();
}
