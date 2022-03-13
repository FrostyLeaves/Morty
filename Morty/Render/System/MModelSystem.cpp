#include "MModelSystem.h"

#include "MScene.h"
#include "MSkeletalAnimation.h"
#include "MModelComponent.h"
#include "MSceneComponent.h"

MORTY_CLASS_IMPLEMENT(MModelSystem, MISystem)

MModelSystem::MModelSystem()
{

}

MModelSystem::~MModelSystem()
{

}

void MModelSystem::SceneTick(MScene* pScene, const float& fDelta)
{
	UpdateAnimation(pScene, fDelta);
}

void MModelSystem::UpdateAnimation(MScene* pScene, const float& fDelta)
{
	bool bVisible = false;
	MSkeletalAnimController* pController = nullptr;
	if (MComponentGroup<MModelComponent>* pModelComponents = pScene->FindComponents<MModelComponent>())
	{
		for (MModelComponent& modelComponent : pModelComponents->m_vComponents)
		{
			if (modelComponent.IsValid())
			{
				if (MSceneComponent* pSceneComponent = modelComponent.GetEntity()->GetComponent<MSceneComponent>())
				{
					bVisible = pSceneComponent->GetVisibleRecursively();
				}
				else
				{
					bVisible = false;
				}

				if (pController = modelComponent.GetSkeletalAnimationController())
				{
					if (pController->GetState() == MIAnimController::EPlay)
					{
						pController->Update(fDelta, bVisible);
					}
				}
			}
		}
	}
}
