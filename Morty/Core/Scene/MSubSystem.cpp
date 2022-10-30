#include "MSubSystem.h"
#include "Scene/MScene.h"

MORTY_INTERFACE_IMPLEMENT(MISubSystem, MTypeClass)

MISubSystem::MISubSystem()
	: m_pScene(nullptr)
{

}

MISubSystem::~MISubSystem()
{

}

void MISubSystem::SetScene(MScene* pScene)
{
	m_pScene = pScene;
}

MScene* MISubSystem::GetScene()
{
	return m_pScene;
}

MEngine* MISubSystem::GetEngine()
{
	if (MScene* pScene = GetScene())
	{
		return pScene->GetEngine();
	}

	return nullptr;
}