#include "MRenderModule.h"
#include "Engine/MEngine.h"
#include "Scene/MEntity.h"
#include "Utility/MFunction.h"

#include "System/MModelSystem.h"
#include "System/MNotifyManager.h"
#include "System/MRenderSystem.h"
#include "System/MSkyBoxSystem.h"
#include "System/MResourceSystem.h"
#include "System/MComponentSystem.h"

#include "Resource/MMeshResource.h"
#include "Resource/MShaderResource.h"
#include "Resource/MEntityResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MSkeletonResource.h"
#include "Resource/MSkeletalAnimationResource.h"

#include "Component/MModelComponent.h"
#include "Component/MCameraComponent.h"
#include "Component/MSpotLightComponent.h"
#include "Component/MPointLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MDebugRenderComponent.h"
#include "Component/MSkyBoxComponent.h"

#include "Shadow/MShadowMeshManager.h"
#include "Manager/MEnvironmentManager.h"

#include "Batch/MMeshInstanceManager.h"
#include "Manager/MAnimationManager.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"

#include "Mesh/MMeshManager.h"

#include "TaskGraph/MTaskGraph.h"

#include "Module/MCoreNotify.h"
#include "Render/MMaterialName.h"
#include "Resource/MMaterialResourceData.h"
#include "Resource/MMaterialTemplateResource.h"
#include "Resource/MTextureResourceUtil.h"

const MString MRenderModule::DefaultWhite = MString("Default_White");
const MString MRenderModule::DefaultNormal = MString("Default_Normal");
const MString MRenderModule::Default_R8_One = MString("Default_R8_One");
const MString MRenderModule::Default_R8_Zero = MString("Default_R8_Zero");
const MString MRenderModule::DefaultAnimationMaterial = MString("Default_Animation_Material");

bool MRenderModule::Register(MEngine* pEngine)
{
	if (!pEngine)
	{
		return false;
	}

	MTaskGraph* pTaskGraph = pEngine->GetMainGraph();

	pEngine->RegisterSystem<MModelSystem>();
	pEngine->RegisterSystem<MSkyBoxSystem>();

	MRenderSystem* pRenderSystem = pEngine->RegisterSystem<MRenderSystem>();

	if (MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>())
	{
		pResourceSystem->RegisterResourceLoader<MMeshResourceLoader>();
		pResourceSystem->RegisterResourceLoader<MShaderResourceLoader>();
		pResourceSystem->RegisterResourceLoader<MTextureResourceLoader>();
		pResourceSystem->RegisterResourceLoader<MMaterialResourceLoader>();
		pResourceSystem->RegisterResourceLoader<MSkeletonResourceLoader>();
		pResourceSystem->RegisterResourceLoader<MSkeletalAnimationLoader>();


		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(DefaultWhite))
		{
			MByte byte[4];
			byte[0] = byte[1] = byte[2] = byte[3] = 255;
 			pTexture->Load(MTextureResourceUtil::LoadFromMemory("Default_White", byte, 1, 1, 4));
		}

		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(DefaultNormal))
		{
			MByte byte[3];
			byte[0] = byte[1] = 127;
			byte[2] = 255;
			pTexture->Load(MTextureResourceUtil::LoadFromMemory("Default_Normal", byte, 1, 1, 3));
		}

		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(Default_R8_One))
		{
			MByte byte = 255;
			pTexture->Load(MTextureResourceUtil::LoadFromMemory("R8_One", & byte, 1, 1, 1));
		}

		if (std::shared_ptr<MTextureResource> pTexture = pResourceSystem->CreateResource<MTextureResource>(Default_R8_Zero))
		{
			MByte byte = 0;
			pTexture->Load(MTextureResourceUtil::LoadFromMemory("R8_Zero", & byte, 1, 1, 1));
		}
	}

	pEngine->RegisterGlobalObject<MMeshManager>();

	if (MComponentSystem* pComponentSystem = pEngine->FindSystem<MComponentSystem>())
	{
		pComponentSystem->RegisterComponent<MModelComponent>();
		pComponentSystem->RegisterComponent<MCameraComponent>();
		pComponentSystem->RegisterComponent<MSpotLightComponent>();
		pComponentSystem->RegisterComponent<MPointLightComponent>();
		pComponentSystem->RegisterComponent<MRenderMeshComponent>();
		pComponentSystem->RegisterComponent<MDirectionalLightComponent>();
		pComponentSystem->RegisterComponent<MDebugRenderComponent>();
		pComponentSystem->RegisterComponent<MSkyBoxComponent>();
	}


	if (MObjectSystem* pObjectSystem = pEngine->FindSystem<MObjectSystem>())
	{
		pObjectSystem->RegisterPostCreateObject(MRenderModule::OnObjectPostCreate);
	}

	MTaskNode* pRenderUpdateTask = pTaskGraph->AddNode<MTaskNode>(MRenderGlobal::TASK_RENDER_MODULE_UPDATE);
	pRenderUpdateTask->SetThreadType(METhreadType::ERenderThread);
	pRenderUpdateTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MRenderSystem::Update, pRenderSystem));


	MRenderModule::RegisterMaterial(pEngine);

	return true;
}

void MRenderModule::OnObjectPostCreate(MObject* pObject)
{
	if (!pObject)
	{
		return;
	}

	if(pObject->GetType() == MScene::GetClassType())
	{
		if (MScene* pScene = pObject->template DynamicCast<MScene>())
		{
			pScene->RegisterManager<MMeshInstanceManager>();
			pScene->RegisterManager<MEnvironmentManager>();
			pScene->RegisterManager<MShadowMeshManager>();
			pScene->RegisterManager<MAnimationManager>();
		}
	}
}

void MRenderModule::RegisterMaterial(MEngine* pEngine)
{
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
	MORTY_ASSERT(pResourceSystem);

	const auto skybox_vs = pResourceSystem->LoadResource("Shader/Environment/skybox.mvs");
	const auto skybox_ps = pResourceSystem->LoadResource("Shader/Environment/skybox.mps");
	const auto skybox_mat = pResourceSystem->CreateResource<MMaterialTemplateResource>(MMaterialName::SKY_BOX);
	skybox_mat->SetCullMode(MECullMode::ECullNone);
	skybox_mat->LoadShader(skybox_vs);
	skybox_mat->LoadShader(skybox_ps);


	const auto universal_vs = pResourceSystem->LoadResource("Shader/Model/universal_model.mvs");
	const auto gbuffer_ps = pResourceSystem->LoadResource("Shader/Deferred/deferred_gbuffer.mps");
	const auto frame_mat = pResourceSystem->CreateResource<MMaterialTemplateResource>(MMaterialName::FRAME_DEFAULT);
	frame_mat->SetCullMode(MECullMode::ECullBack);
	frame_mat->LoadShader(universal_vs);
	frame_mat->LoadShader(gbuffer_ps);

	const auto basic_ps = pResourceSystem->LoadResource("Shader/Forward/basic_lighting.mps");
	const auto basic_mat = pResourceSystem->CreateResource<MMaterialTemplateResource>(MMaterialName::BASIC_LIGHTING);
	basic_mat->SetCullMode(MECullMode::ECullBack);
	basic_mat->LoadShader(universal_vs);
	basic_mat->LoadShader(basic_ps);

	const auto basic_ske_mat = pResourceSystem->CreateResource<MMaterialTemplateResource>(MMaterialName::BASIC_LIGHTING_SKELETON);
	basic_ske_mat->AddDefine(MRenderGlobal::SHADER_SKELETON_ENABLE, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	basic_ske_mat->SetCullMode(MECullMode::ECullBack);
	basic_ske_mat->LoadShader(universal_vs);
	basic_ske_mat->LoadShader(gbuffer_ps);

	const auto gbuffer_mat = pResourceSystem->CreateResource<MMaterialTemplateResource>(MMaterialName::DEFERRED_GBUFFER);
	gbuffer_mat->AddDefine(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	gbuffer_mat->LoadShader(universal_vs);
	gbuffer_mat->LoadShader(gbuffer_ps);
	gbuffer_mat->SetMaterialType(MEMaterialType::EDeferred);

	const auto gbuffer_ske_mat = pResourceSystem->CreateResource<MMaterialTemplateResource>(MMaterialName::DEFERRED_GBUFFER_SKELETON);
	gbuffer_ske_mat->AddDefine(MRenderGlobal::SHADER_SKELETON_ENABLE, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	gbuffer_ske_mat->AddDefine(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	gbuffer_ske_mat->LoadShader(universal_vs);
	gbuffer_ske_mat->LoadShader(gbuffer_ps);
	gbuffer_ske_mat->SetMaterialType(MEMaterialType::EDeferred);

	const auto deferred_vs = pResourceSystem->LoadResource("Shader/Deferred/deferred_lighting.mvs");
	const auto deferred_ps = pResourceSystem->LoadResource("Shader/Deferred/deferred_lighting.mps");
	const auto deferred_mat = pResourceSystem->CreateResource<MMaterialTemplateResource>(MMaterialName::DEFERRED_LIGHTING);
	deferred_mat->LoadShader(deferred_vs);
	deferred_mat->LoadShader(deferred_ps);


	const auto shadowmap_vs = pResourceSystem->LoadResource("Shader/Shadow/shadowmap.mvs");
	const auto shadowmap_ps = pResourceSystem->LoadResource("Shader/Shadow/shadowmap.mps");
	const auto shadowmap_mat = pResourceSystem->CreateResource<MMaterialTemplateResource>(MMaterialName::SHADOW_MAP);
	shadowmap_mat->SetCullMode(MECullMode::ECullNone);
	shadowmap_mat->AddDefine(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	shadowmap_mat->LoadShader(shadowmap_vs);
	shadowmap_mat->LoadShader(shadowmap_ps);

	const auto shadowmap_ske_mat = pResourceSystem->CreateResource<MMaterialTemplateResource>(MMaterialName::SHADOW_MAP_SKELETON);
	shadowmap_ske_mat->SetCullMode(MECullMode::ECullNone);
	shadowmap_ske_mat->AddDefine(MRenderGlobal::DRAW_MESH_INSTANCING_STORAGE, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	shadowmap_ske_mat->AddDefine(MRenderGlobal::SHADER_SKELETON_ENABLE, MRenderGlobal::SHADER_DEFINE_ENABLE_FLAG);
	shadowmap_ske_mat->LoadShader(shadowmap_vs);
	shadowmap_ske_mat->LoadShader(shadowmap_ps);


}
