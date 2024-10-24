#include "MManager.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(IManager, MTypeClass)

IManager::IManager()
    : m_scene(nullptr)
{}

IManager::~IManager() {}

void     IManager::SetScene(MScene* pScene) { m_scene = pScene; }

MScene*  IManager::GetScene() { return m_scene; }

MEngine* IManager::GetEngine()
{
    if (MScene* pScene = GetScene()) { return pScene->GetEngine(); }

    return nullptr;
}