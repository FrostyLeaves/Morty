#include "MComputeDispatcher.h"

#include "Engine/MEngine.h"
#include "RHI/Abstract/MIDevice.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MShaderResource.h"
#include "Shader/MShader.h"
#include <utility>

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MComputeDispatcher, MObject)

bool MComputeDispatcher::LoadComputeShader(const std::shared_ptr<MResource>& resource, const MStringId& entryName)
{
    MEntryNames entryNames;
    entryNames[(size_t) MEShaderType::ECompute] = entryName;

    m_shaderProgram = std::make_unique<MShaderProgram>(
            GetEngine(),
            MShaderProgram::EUsage::ECompute,
            resource,
            MShaderMacro(),
            entryNames,
            MShaderUsageMask::Compute
    );

    return m_shaderProgram->IsValid();
}

bool MComputeDispatcher::LoadComputeShader(const MString& strResource, const MStringId& entryName)
{
    auto resourceSystem = GetEngine()->FindSystem<MResourceSystem>();
    if (std::shared_ptr<MResource> pResource = resourceSystem->LoadResource(strResource))
        return LoadComputeShader(pResource, entryName);

    return false;
}

MShader* MComputeDispatcher::GetComputeShader()
{
    if (m_shaderProgram) return m_shaderProgram->GetShader(MEShaderType::ECompute);
    return nullptr;
}

void MComputeDispatcher::OnCreated() { Super::OnCreated(); }

void MComputeDispatcher::OnDelete()
{
    m_shaderProgram = nullptr;

    Super::OnDelete();
}
