#include "MManager.h"
#include "Scene/MScene.h"

using namespace morty;

MORTY_INTERFACE_IMPLEMENT(IManager, MTypeClass)

IManager::IManager()
    : m_scene(nullptr)
{}

IManager::~IManager() {}

void     IManager::SetScene(MScene* scene) { m_scene = scene; }

MScene*  IManager::GetScene() { return m_scene; }

MEngine* IManager::GetEngine()
{
    if (MScene* scene = GetScene()) { return scene->GetEngine(); }

    return nullptr;
}