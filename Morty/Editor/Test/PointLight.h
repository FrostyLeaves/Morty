#include "MRenderModule.h"
#include "Component/MPointLightComponent.h"
#include "Scene/MScene.h"
#include "Engine/MEngine.h"

#include "System/MEntitySystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MRenderMeshComponent.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"

void ADD_POINT_LIGHT(MEngine* pEngine, MScene* pScene)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	std::shared_ptr<MResource> pIconTexture = pResourceSystem->LoadResource("Texture/Icon/point_light.png");

	std::shared_ptr<MMeshResource> pPanelMesh = pResourceSystem->CreateResource<MMeshResource>();
	pPanelMesh->LoadAsPlane(MMeshResource::MEMeshVertexType::Normal, Vector3(10.0f, 10.0f, 1.0f));

	std::shared_ptr<MMaterialResource> pMaterial = pResourceSystem->CreateResource<MMaterialResource>();

	pMaterial->LoadVertexShader("Shader/Debug/debug_model.mvs");
	pMaterial->LoadPixelShader("Shader/Debug/debug_model.mps");

	pMaterial->SetTexture("u_texDiffuse", pIconTexture);

	for (int i = 0; i < 9; ++i)
	{
		MString strEntityName = "PointLight_" + MStringHelper::ToString(i);

		MEntity* pPointLight = pScene->CreateEntity();
		pPointLight->SetName(strEntityName);

		if (MSceneComponent* pSceneComponent = pPointLight->RegisterComponent<MSceneComponent>())
		{
			pSceneComponent->SetPosition(Vector3(i / 3 * 10.0f, 10.0f, i % 3 * 10.0f));
		}

		if (MPointLightComponent* pPointLightComponent = pPointLight->RegisterComponent<MPointLightComponent>())
		{
			pPointLightComponent->SetLightIntensity(100.0f);
		}

		if (MRenderMeshComponent* pMeshComponent = pPointLight->RegisterComponent<MRenderMeshComponent>())
		{
			pMeshComponent->Load(pPanelMesh);
			pMeshComponent->SetMaterial(pMaterial);
		}
	}
}
