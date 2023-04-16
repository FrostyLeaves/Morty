#include "MManager.h"
#include "Scene/MScene.h"

MORTY_INTERFACE_IMPLEMENT(IManager, MTypeClass)

IManager::IManager()
	: m_pScene(nullptr)
{

}

IManager::~IManager()
{

}

void IManager::SetScene(MScene* pScene)
{
	m_pScene = pScene;
}

MScene* IManager::GetScene()
{
	return m_pScene;
}

MEngine* IManager::GetEngine()
{
	if (MScene* pScene = GetScene())
	{
		return pScene->GetEngine();
	}

	return nullptr;
}