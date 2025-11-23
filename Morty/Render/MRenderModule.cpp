#include "MRenderModule.h"
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
#include "Resource/MMeshResource.h"
#include "Resource/MShaderResource.h"
#include "Resource/MSkeletalAnimationResource.h"
#include "Resource/MSkeletonResource.h"
#include "Resource/MTextureResource.h"
#include "Resource/MTextureResourceUtil.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "System/MComponentSystem.h"
#include "System/MModelSystem.h"
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

bool          MRenderModule::Register(MEngine* pEngine)
{
    if (!pEngine) { return false; }

    MTaskGraph* pTaskGraph = pEngine->GetMainGraph();

    pEngine->RegisterSystem<MModelSystem>();
    pEngine->RegisterSystem<MSkyBoxSystem>();
    pEngine->RegisterSystem<MShaderProgramSystem>();

    MRenderSystem* pRenderSystem = pEngine->RegisterSystem<MRenderSystem>();

    if (MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>())
    {
        pResourceSystem->RegisterResourceLoader<MMeshResourceLoader>();
        pResourceSystem->RegisterResourceLoader<MShaderResourceLoader>();
        pResourceSystem->RegisterResourceLoader<MTextureResourceLoader>();
        pResourceSystem->RegisterResourceLoader<MMaterialResourceLoader>();
        pResourceSystem->RegisterResourceLoader<MSkeletonResourceLoader>();
        pResourceSystem->RegisterResourceLoader<MSkeletalAnimationLoader>();


        if (std::shared_ptr<MTextureResource> pTexture =
                    pResourceSystem->CreateResource<MTextureResource>(DefaultWhite))
        {
            std::vector<MByte> byte(4);
            byte[0] = byte[1] = byte[2] = byte[3] = 255;
            pTexture->Load(MTextureResourceUtil::LoadFromMemory(DefaultWhite, byte, 1, 1, 4, MTexturePixelType::Byte8));
        }
        if (std::shared_ptr<MTextureResource> pTexture =
                    pResourceSystem->CreateResource<MTextureResource>(DefaultBlack))
        {
            std::vector<MByte> byte(4);
            byte[0] = byte[1] = byte[2] = byte[3] = 0;
            pTexture->Load(MTextureResourceUtil::LoadFromMemory(DefaultBlack, byte, 1, 1, 4, MTexturePixelType::Byte8));
        }

        if (std::shared_ptr<MTextureResource> pTexture =
                    pResourceSystem->CreateResource<MTextureResource>(DefaultNormal))
        {
            std::vector<MByte> byte(3);
            byte[0] = byte[1] = 127;
            byte[2]           = 255;
            pTexture->Load(
                    MTextureResourceUtil::LoadFromMemory("Default_Normal", byte, 1, 1, 3, MTexturePixelType::Byte8)
            );
        }

        if (std::shared_ptr<MTextureResource> pTexture =
                    pResourceSystem->CreateResource<MTextureResource>(Default_R8_One))
        {
            std::vector<MByte> byte(1);
            byte[0] = 255;
            pTexture->Load(MTextureResourceUtil::LoadFromMemory("R8_One", byte, 1, 1, 1, MTexturePixelType::Byte8));
        }

        if (std::shared_ptr<MTextureResource> pTexture =
                    pResourceSystem->CreateResource<MTextureResource>(Default_R8_Zero))
        {
            std::vector<MByte> byte(1);
            byte[0] = 0;
            pTexture->Load(MTextureResourceUtil::LoadFromMemory("R8_Zero", byte, 1, 1, 1, MTexturePixelType::Byte8));
        }

        if (std::shared_ptr<MTextureResource> pTexture =
                    pResourceSystem->CreateResource<MTextureResource>(NoiseTexture))
        {
            constexpr size_t   nSize = 8;
            std::vector<MByte> byte(nSize * nSize * 4);
            for (size_t nIdx = 0; nIdx < nSize * 4; ++nIdx) { byte[nIdx] = MMath::RandInt(0, 255); }
            pTexture->Load(
                    MTextureResourceUtil::LoadFromMemory(NoiseTexture, byte, nSize, nSize, 4, MTexturePixelType::Byte8)
            );
        }
    }

    pEngine->RegisterGlobalObject<MMeshManager>();

    if (auto pComponentSystem = pEngine->FindSystem<MComponentSystem>())
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
    MORTY_UNUSED(pObject);
    /*
    if (!pObject) { return; }

    if (pObject->GetType() == MScene::GetClassType())
    {
        if (MScene* pScene = pObject->template DynamicCast<MScene>())
        {
            //pScene->RegisterManager<MMeshInstanceManager>();
            //pScene->RegisterManager<MEnvironmentManager>();
            //pScene->RegisterManager<MShadowMeshManager>();
            //pScene->RegisterManager<MAnimationManager>();
        }
    }
    */
}

void MRenderModule::RegisterMaterial(MEngine* pEngine)
{
    MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();
    MORTY_ASSERT(pResourceSystem);
}
