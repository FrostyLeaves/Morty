#include "MaterialView.h"

#include "Basic/MTexture.h"
#include "Basic/MViewport.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Object/MObject.h"
#include "Resource/MMaterialResource.h"
#include "Scene/MEntity.h"
#include "Scene/MScene.h"

#include "imgui.h"

#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"
#include "System/MSceneSystem.h"

#include "Component/MDirectionalLightComponent.h"
#include "Component/MModelComponent.h"
#include "Component/MRenderMeshComponent.h"
#include "Component/MSceneComponent.h"

#include "Resource/MSkeletonResource.h"

#include "RenderProgram/MIRenderProgram.h"

#include "Main/MainEditor.h"
#include "Resource/MMeshResourceUtil.h"
#include "Utility/SelectionEntityManager.h"

using namespace morty;

MaterialView::MaterialView()
    : BaseWidget()
{
    m_strViewName = "Material";
}

void MaterialView::SetMaterial(std::shared_ptr<MMaterialResource> pMaterial)
{
    if (m_material == pMaterial) return;

    MSceneSystem* pSceneSystem = GetEngine()->FindSystem<MSceneSystem>();

    m_material = pMaterial;

    if (!m_material)
    {
        pSceneSystem->SetVisible(m_staticSphereMeshNode, false);
        pSceneSystem->SetVisible(m_skeletonSphereMeshNode, false);
    }
    else if (m_material->GetShaderMacro()
                     .GetMacro(MRenderGlobal::SHADER_SKELETON_ENABLE)
                     .empty())
    {
        pSceneSystem->SetVisible(m_staticSphereMeshNode, true);
        pSceneSystem->SetVisible(m_skeletonSphereMeshNode, false);

        if (MRenderMeshComponent* pMeshComponent =
                    m_staticSphereMeshNode->GetComponent<MRenderMeshComponent>())
        {
            pMeshComponent->SetMaterial(pMaterial);
        }
    }
    else
    {
        pSceneSystem->SetVisible(m_staticSphereMeshNode, false);
        pSceneSystem->SetVisible(m_skeletonSphereMeshNode, true);

        if (MRenderMeshComponent* pMeshComponent =
                    m_skeletonSphereMeshNode->GetComponent<MRenderMeshComponent>())
        {
            pMeshComponent->SetMaterial(pMaterial);
        }
    }
}

void MaterialView::Render()
{
    if (MEntity* pEntity = SelectionEntityManager::GetInstance()->GetSelectedEntity())
    {
        if (MRenderMeshComponent* pMeshComponent =
                    pEntity->GetComponent<MRenderMeshComponent>())
        {
            SetMaterial(pMeshComponent->GetMaterialResource());
        }
    }

    if (m_material)
    {
        if (std::shared_ptr<MTexture> pTexture = m_sceneTexture->GetTexture())
        {
            float fImageSize = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(fImageSize * 0.25f);
            ImGui::Image(
                    {pTexture, intptr_t(pTexture.get()), 0},
                    ImVec2(fImageSize * 0.5f, fImageSize * 0.5f)
            );
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
        ImGui::Columns(2);
        ImGui::Separator();

        const bool bModify = m_propertyBase.EditMMaterial(m_material);
        m_sceneTexture->SetPauseUpdate(!bModify);


        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::PopStyleVar();
    }
    else { m_sceneTexture->SetPauseUpdate(true); }
}

void MaterialView::Initialize(MainEditor* pMainEditor)
{
    BaseWidget::Initialize(pMainEditor);

    MSceneSystem*    pSceneSystem    = GetEngine()->FindSystem<MSceneSystem>();
    MObjectSystem*   pObjectSystem   = GetEngine()->FindSystem<MObjectSystem>();
    MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    m_scene = pObjectSystem->CreateObject<MScene>();

    m_sceneTexture = GetMainEditor()->CreateSceneViewer(m_scene);
    m_sceneTexture->SetRect(Vector2i(0, 0), Vector2i(512, 512));

    if (MEntity* pCameraNode = m_sceneTexture->GetViewport()->GetCamera())
    {
        if (MSceneComponent* pCameraSceneComponent =
                    pCameraNode->GetComponent<MSceneComponent>())
        {
            pCameraSceneComponent->SetPosition(Vector3(0, 0, -5));
            pCameraSceneComponent->SetRotation(UnitQuaternion);
        }
    }

    m_staticSphereMeshNode = m_scene->CreateEntity();

    if (MRenderMeshComponent* pMeshComponent =
                m_staticSphereMeshNode->RegisterComponent<MRenderMeshComponent>())
    {
        std::shared_ptr<MMeshResource> pMeshResource =
                pResourceSystem->CreateResource<MMeshResource>();
        pMeshResource->Load(MMeshResourceUtil::CreateSphere());
        pMeshComponent->Load(pMeshResource);
    }

    pSceneSystem->SetVisible(m_staticSphereMeshNode, false);

    m_skeletonSphereMeshNode = m_scene->CreateEntity();

    if (MModelComponent* pModelComponent =
                m_skeletonSphereMeshNode->RegisterComponent<MModelComponent>())
    {
        std::shared_ptr<MSkeletonResource> pSkeleton =
                pResourceSystem->CreateResource<MSkeletonResource>();
        pModelComponent->SetSkeletonResource(pSkeleton);
    }

    if (MRenderMeshComponent* pMeshComponent =
                m_skeletonSphereMeshNode->RegisterComponent<MRenderMeshComponent>())
    {
        std::shared_ptr<MMeshResource> pMeshResource =
                pResourceSystem->CreateResource<MMeshResource>();
        pMeshResource->Load(MMeshResourceUtil::CreateSphere(MEMeshVertexType::Skeleton));
        pMeshComponent->Load(pMeshResource);
    }

    pSceneSystem->SetVisible(m_skeletonSphereMeshNode, false);


    MEntity* pDirLight = m_scene->CreateEntity();
    pDirLight->SetName("DirLight");

    if (MSceneComponent* pDirLightSceneComponent =
                pDirLight->RegisterComponent<MSceneComponent>())
    {
        Quaternion quat;
        quat.SetEulerAngle(Vector3(-45, 45, 0));
        pDirLightSceneComponent->SetRotation(Quaternion(quat));
    }

    if (MDirectionalLightComponent* pDirLightComponent =
                pDirLight->RegisterComponent<MDirectionalLightComponent>())
    {
        pDirLightComponent->SetLightIntensity(10.0f);
    }
}

void MaterialView::Release()
{
    SetMaterial(nullptr);

    m_scene->DeleteEntity(m_skeletonSphereMeshNode);
    m_skeletonSphereMeshNode = nullptr;
    m_scene->DeleteEntity(m_staticSphereMeshNode);
    m_staticSphereMeshNode = nullptr;
    m_scene->DeleteLater();
    m_scene = nullptr;

    GetMainEditor()->DestroySceneViewer(m_sceneTexture);
    m_sceneTexture = nullptr;
}

void MaterialView::Input(MInputEvent* pEvent)
{
    m_sceneTexture->GetViewport()->Input(pEvent);
}
