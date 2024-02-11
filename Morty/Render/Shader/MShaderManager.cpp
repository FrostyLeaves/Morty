#include "MShaderManager.h"

#include "Engine/MEngine.h"
#include "Mesh/MMeshUtil.h"
#include "Render/MIDevice.h"
#include "Render/MMesh.h"
#include "Render/MVertex.h"
#include "Resource/MMeshResource.h"
#include "Resource/MMeshResourceUtil.h"
#include "System/MRenderSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"
#include "Utility/MGlobal.h"
#include "RenderProgram/RenderWork/MRenderWork.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MShaderManager, MObject)

MShaderManager::MShaderManager()
{
}

void MShaderManager::OnCreated()
{
}

void MShaderManager::OnDelete()
{
}

std::shared_ptr<MShaderProgram> MShaderManager::FindOrCreateShaderProgram(MShaderProgramKey key)
{
    MORTY_UNUSED(key);
    return std::shared_ptr<MShaderProgram>();
}
