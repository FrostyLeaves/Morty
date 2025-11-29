#include "MShaderProgramSystem.h"
#include "Utility/MGlobal.h"
#include "Engine/MEngine.h"
#include "Material/MMaterialPass.h"
#include "Material/MMaterialTemplate.h"
#include "Mesh/MMesh.h"
#include "Mesh/MMeshUtil.h"
#include "Mesh/MVertex.h"
#include "RHI/Abstract/MIDevice.h"
#include "Render/RenderGraph/MRenderCommon.h"
#include "Resource/MMeshResource.h"
#include "Resource/MMeshResourceUtil.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"


using namespace morty;

MORTY_CLASS_IMPLEMENT(MShaderProgramSystem, MISystem)

MShaderProgram* MShaderProgramSystem::FindShaderProgram(const MMaterialPass* pass)
{
    auto findResult = m_materialPassTable.find(pass);
    if (findResult != m_materialPassTable.end()) { return findResult->second->program.get(); }

    return nullptr;
}

MShaderProgram* MShaderProgramSystem::CreateShaderProgram(const MMaterialPass* pass)
{
    {
        auto findResult = m_materialPassTable.find(pass);
        if (findResult != m_materialPassTable.end()) { return findResult->second->program.get(); }
    }

    std::shared_ptr<MShaderProgramReference> reference;

    {
        const MShaderProgramKey key        = pass->GetHashCode();
        auto                    findResult = m_shaderProgramTable.find(key);
        if (findResult == m_shaderProgramTable.end())
        {
            reference      = std::make_shared<MShaderProgramReference>();
            reference->key = pass->GetHashCode();
        }
        else
        {
            reference = findResult->second;
            MORTY_ASSERT(reference->key == pass->GetHashCode());
        }
    }

    reference->owner.insert(pass);
    m_materialPassTable[pass] = reference;

    if (reference->program == nullptr)
    {
        auto materialTemplate = pass->GetTemplate();
        reference->program    = std::make_unique<MShaderProgram>(
                GetEngine(),
                IShaderProgram::EUsage::EGraphics,
                materialTemplate->GetShaderResource(),
                materialTemplate->GetShaderMacro(),
                pass->GetEntryNames(),
                MShaderUsageMask::Vertex | MShaderUsageMask::Pixel
        );
    }

    return reference->program.get();
}

void MShaderProgramSystem::ReleaseShaderProgram(const MMaterialPass* pass)
{
    auto findResult = m_materialPassTable.find(pass);
    if (findResult == m_materialPassTable.end()) { return; }

    auto reference = findResult->second;
    m_materialPassTable.erase(findResult);

    if (nullptr == reference) { return; }

    reference->owner.erase(pass);
    if (reference->owner.empty())
    {
        reference->program = nullptr;
        m_shaderProgramTable.erase(reference->key);
    }
}

void MShaderProgramSystem::Release()
{
    m_shaderProgramTable.clear();
    m_materialPassTable.clear();
}