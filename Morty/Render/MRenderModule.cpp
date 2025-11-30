#include "MRenderModule.h"
#include "Batch/Mesh/MMeshInstanceManager.h"
#include "Component/MCameraComponent.h"
#include "Component/MDebugRenderComponent.h"
#include "Component/MDirectionalLightComponent.h"
#include "Component/MModelComponent.h"
#include "Component/MPointLightComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSkyBoxComponent.h"
#include "Component/MSpotLightComponent.h"
#include "Engine/MEngine.h"
#include "Mesh/MMeshManager.h"
#include "Module/MCoreNotify.h"
#include "Resource/MEntityResource.h"
#include "Resource/MMaterialResource.h"
#include "Resource/MMaterialResourceData.h"
#include "Resource/MMaterialTemplateResource.h"
#include "Resource/MMaterialTemplateResourceData.h"
#include "Resource/MMeshResource.h"
#include "Resource/MShaderResource.h"
#include "Resource/MSkeletalAnimationResource.h"
#include "Resource/MSkeletonResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MTextureResourceUtil.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "System/MComponentSystem.h"
#include "System/MNotifyManager.h"
#include "System/MObjectSystem.h"
#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"
#include "System/MShaderProgramSystem.h"
#include "System/MSkyBoxSystem.h"
#include "TaskGraph/MTaskGraph.h"
#include "Utility/MFunction.h"
#include "Utility/MMaterialName.h"


using namespace morty;

const MString MRenderModule::DefaultWhite             = MString("Default_White");
const MString MRenderModule::DefaultBlack             = MString("Default_Black");
const MString MRenderModule::DefaultNormal            = MString("Default_Normal");
const MString MRenderModule::Default_R8_One           = MString("Default_R8_One");
const MString MRenderModule::Default_R8_Zero          = MString("Default_R8_Zero");
const MString MRenderModule::DefaultAnimationMaterial = MString("Default_Animation_Material");
const MString MRenderModule::NoiseTexture             = MString("Noise Texture");

bool          MRenderModule::Register(MEngine* engine)
{
    if (!engine) { return false; }

    MTaskGraph* pTaskGraph = engine->GetMainGraph();

    engine->RegisterSystem<MSkyBoxSystem>();
    engine->RegisterSystem<MShaderProgramSystem>();

    auto renderSystem = engine->RegisterSystem<MRenderSystem>();

    if (auto resourceSystem = engine->FindSystem<MResourceSystem>())
    {
        resourceSystem->RegisterResourceLoader<MMeshResourceLoader>();
        resourceSystem->RegisterResourceLoader<MShaderResourceLoader>();
        resourceSystem->RegisterResourceLoader<MTextureResourceLoader>();
        resourceSystem->RegisterResourceLoader<MMaterialResourceLoader>();
        resourceSystem->RegisterResourceLoader<MSkeletonResourceLoader>();
        resourceSystem->RegisterResourceLoader<MSkeletalAnimationLoader>();
        resourceSystem->RegisterResourceLoader<MMaterialTemplateResourceDataLoader>();


        if (std::shared_ptr<MTextureResource> texture = resourceSystem->CreateResource<MTextureResource>(DefaultWhite))
        {
            std::vector<MByte> byte(4);
            byte[0] = byte[1] = byte[2] = byte[3] = 255;
            texture->Load(MTextureResourceUtil::LoadFromMemory(DefaultWhite, byte, 1, 1, 4, MTexturePixelType::Byte8));
        }
        if (std::shared_ptr<MTextureResource> texture = resourceSystem->CreateResource<MTextureResource>(DefaultBlack))
        {
            std::vector<MByte> byte(4);
            byte[0] = byte[1] = byte[2] = byte[3] = 0;
            texture->Load(MTextureResourceUtil::LoadFromMemory(DefaultBlack, byte, 1, 1, 4, MTexturePixelType::Byte8));
        }

        if (std::shared_ptr<MTextureResource> texture = resourceSystem->CreateResource<MTextureResource>(DefaultNormal))
        {
            std::vector<MByte> byte(3);
            byte[0] = byte[1] = 127;
            byte[2]           = 255;
            texture->Load(
                    MTextureResourceUtil::LoadFromMemory("Default_Normal", byte, 1, 1, 3, MTexturePixelType::Byte8)
            );
        }

        if (std::shared_ptr<MTextureResource> texture =
                    resourceSystem->CreateResource<MTextureResource>(Default_R8_One))
        {
            std::vector<MByte> byte(1);
            byte[0] = 255;
            texture->Load(MTextureResourceUtil::LoadFromMemory("R8_One", byte, 1, 1, 1, MTexturePixelType::Byte8));
        }

        if (std::shared_ptr<MTextureResource> texture =
                    resourceSystem->CreateResource<MTextureResource>(Default_R8_Zero))
        {
            std::vector<MByte> byte(1);
            byte[0] = 0;
            texture->Load(MTextureResourceUtil::LoadFromMemory("R8_Zero", byte, 1, 1, 1, MTexturePixelType::Byte8));
        }

        if (std::shared_ptr<MTextureResource> texture = resourceSystem->CreateResource<MTextureResource>(NoiseTexture))
        {
            constexpr size_t   nSize = 8;
            std::vector<MByte> byte(nSize * nSize * 4);
            for (size_t nIdx = 0; nIdx < nSize * 4; ++nIdx) { byte[nIdx] = MMath::RandInt(0, 255); }
            texture->Load(
                    MTextureResourceUtil::LoadFromMemory(NoiseTexture, byte, nSize, nSize, 4, MTexturePixelType::Byte8)
            );
        }
    }

    engine->RegisterGlobalObject<MMeshManager>();

    if (auto pComponentSystem = engine->FindSystem<MComponentSystem>())
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


    if (auto objectSystem = engine->FindSystem<MObjectSystem>())
    {
        objectSystem->RegisterPostCreateObject(MRenderModule::OnObjectPostCreate);
    }

    auto renderUpdateTask = pTaskGraph->AddNode<MTaskNode>(MRenderGlobal::TASK_RENDER_MODULE_UPDATE);
    renderUpdateTask->SetThreadType(METhreadType::ERenderThread);
    renderUpdateTask->BindTaskFunction(M_CLASS_FUNCTION_BIND_0_1(MRenderSystem::Update, renderSystem));


    MRenderModule::RegisterMaterial(engine);

    return true;
}

void MRenderModule::OnObjectPostCreate(MObject* pObject)
{
    if (!pObject) { return; }

    if (pObject->GetType() == MScene::GetClassType())
    {
        if (MScene* scene = pObject->template DynamicCast<MScene>())
        {
            scene->RegisterManager<MMeshInstanceManager>();
            //scene->RegisterManager<MEnvironmentManager>();
            //scene->RegisterManager<MShadowMeshManager>();
            //scene->RegisterManager<MAnimationManager>();
        }
    }
}

void MRenderModule::RegisterMaterial(MEngine* engine)
{
    MResourceSystem* resourceSystem = engine->FindSystem<MResourceSystem>();
    MORTY_ASSERT(resourceSystem);
}
