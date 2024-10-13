#include "MShaderManager.h"

#include "Utility/MGlobal.h"
#include "Engine/MEngine.h"
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

MORTY_CLASS_IMPLEMENT(MShaderManager, MObject)

MShaderManager::MShaderManager() {}

void                            MShaderManager::OnCreated() {}

void                            MShaderManager::OnDelete() {}

std::shared_ptr<MShaderProgram> MShaderManager::FindOrCreateShaderProgram(MShaderProgramKey key)
{
    MORTY_UNUSED(key);
    return std::shared_ptr<MShaderProgram>();
}
