#include "MRenderModule.h"
#include "MEngine.h"
#include "MEntity.h"
#include "MFunction.h"

#include "MNotifySystem.h"
#include "MRenderSystem.h"
#include "MResourceSystem.h"
#include "MComponentSystem.h"

#include "MMeshResource.h"
#include "MShaderResource.h"
#include "MEntityResource.h"
#include "MTextureResource.h"
#include "MMaterialResource.h"
#include "MSkeletonResource.h"
#include "MSkeletalAnimationResource.h"

#include "MModelComponent.h"
#include "MCameraComponent.h"
#include "MSpotLightComponent.h"
#include "MPointLightComponent.h"
#include "MRenderableMeshComponent.h"
#include "MDirectionalLightComponent.h"

#include "MTaskGraph.h"

bool MRenderModule::Register(MEngine* pEngine)
{
	if (!pEngine)
		return false;

	MNotifySystem* pNotifySystem = pEngine->FindSystem<MNotifySystem>();

	if (MRenderSystem* pRenderSystem = pEngine->RegisterSystem<MRenderSystem>())
	{
		pNotifySystem->RegisterNotify("TransformDirty", M_CLASS_FUNCTION_BIND_1(MRenderSystem::OnTransformDirty, pRenderSystem));

		if (MTaskGraph* pTaskGraph = pEngine->GetMainGraph())
		{
			if (MTaskNode* pTaskNode = pTaskGraph->AddNode<MTaskNode>("Render_Update"))
			{
				pTaskNode->SetThreadType(METhreadType::ERenderThread);
				pTaskNode->BindTaskFunction(M_CLASS_FUNCTION_BIND_1(MRenderSystem::Update, pRenderSystem));
			}
		}
	}

	if (MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>())
	{
		pResourceSystem->RegisterResourceType<MMeshResource>();
		pResourceSystem->RegisterResourceType<MShaderResource>();
		pResourceSystem->RegisterResourceType<MEntityResource>();
		pResourceSystem->RegisterResourceType<MTextureResource>();
		pResourceSystem->RegisterResourceType<MMaterialResource>();
		pResourceSystem->RegisterResourceType<MSkeletonResource>();
		pResourceSystem->RegisterResourceType<MSkeletalAnimationResource>();


		if (MTextureResource* pTexture = pResourceSystem->CreateResource<MTextureResource>("White_Texture"))
		{
			MByte byte[4];
			byte[0] = byte[1] = byte[2] = byte[3] = 255;
			pTexture->LoadFromMemory(byte, 1, 1, 32);
		}
	}

	if (MComponentSystem* pComponentSystem = pEngine->FindSystem<MComponentSystem>())
	{
		pComponentSystem->RegisterComponent<MModelComponent>();
		pComponentSystem->RegisterComponent<MCameraComponent>();
		pComponentSystem->RegisterComponent<MSpotLightComponent>();
		pComponentSystem->RegisterComponent<MPointLightComponent>();
		pComponentSystem->RegisterComponent<MRenderableMeshComponent>();
		pComponentSystem->RegisterComponent<MDirectionalLightComponent>();
	}
	

	return true;
}