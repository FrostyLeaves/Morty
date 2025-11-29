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

#include "Render/MIRenderProgram.h"

#include "Main/MainEditor.h"
#include "Resource/MMeshResourceUtil.h"
#include "Utility/NotifyManager.h"
#include "Utility/SelectionContext.h"

using namespace morty;

MaterialView::MaterialView()
    : BaseWidget()
{
    m_strViewName = "Material";
}

void MaterialView::SetMaterial(std::shared_ptr<MMaterialResource> material)
{
    if (m_material == material) return;

    auto* sceneSystem = GetEngine()->FindSystem<MSceneSystem>();

    m_material = material;

    if (!m_material || !m_material->GetTemplate())
    {
        sceneSystem->SetVisible(m_staticSphereMeshNode, false);
        sceneSystem->SetVisible(m_skeletonSphereMeshNode, false);
    }
    else if (m_material->GetShaderMacro().GetMacro(MRenderGlobal::SHADER_SKELETON_ENABLE).empty())
    {
        sceneSystem->SetVisible(m_staticSphereMeshNode, true);
        sceneSystem->SetVisible(m_skeletonSphereMeshNode, false);

        if (auto* meshComponent = m_staticSphereMeshNode->GetComponent<MRenderMeshComponent>())
        {
            meshComponent->SetMaterial(material);
        }
    }
    else
    {
        sceneSystem->SetVisible(m_staticSphereMeshNode, false);
        sceneSystem->SetVisible(m_skeletonSphereMeshNode, true);

        if (auto* meshComponent = m_skeletonSphereMeshNode->GetComponent<MRenderMeshComponent>())
        {
            meshComponent->SetMaterial(material);
        }
    }
}

void MaterialView::Render()
{
    if (m_material)
    {
        m_propertyBase.BindEngine(GetEngine());

        ImGui::Text("%s", m_material->GetResourcePath().c_str());
        ImGui::Separator();

        if (MTexturePtr texture = m_sceneTexture->GetFinalOutputTexture())
        {
            float fImageSize = ImGui::GetContentRegionAvail().x;
            ImGui::SameLine(fImageSize * 0.25f);
            ImGui::Image({texture, intptr_t(texture.get()), 0}, ImVec2(fImageSize * 0.5f, fImageSize * 0.5f));
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

    auto* sceneSystem    = GetEngine()->FindSystem<MSceneSystem>();
    auto* objectSystem   = GetEngine()->FindSystem<MObjectSystem>();
    auto* resourceSystem = GetEngine()->FindSystem<MResourceSystem>();

    m_scene = objectSystem->CreateObject<MScene>();

    m_sceneTexture = GetMainEditor()->CreateSceneViewer("EditorMaterial", m_scene);
    m_sceneTexture->SetRect(Vector2i(0, 0), Vector2i(512, 512));

    if (MEntity* pCameraNode = m_sceneTexture->GetViewport()->GetCamera())
    {
        if (auto* pCameraSceneComponent = pCameraNode->GetComponent<MSceneComponent>())
        {
            pCameraSceneComponent->SetPosition(Vector3(0, 0, -5));
            pCameraSceneComponent->SetRotation(UnitQuaternion);
        }
    }

    m_staticSphereMeshNode = m_scene->CreateEntity();

    if (auto* meshComponent = m_staticSphereMeshNode->RegisterComponent<MRenderMeshComponent>())
    {
        std::shared_ptr<MMeshResource> pMeshResource = resourceSystem->CreateResource<MMeshResource>();
        pMeshResource->Load(MMeshResourceUtil::CreateSphere());
        meshComponent->Load(pMeshResource);
    }

    sceneSystem->SetVisible(m_staticSphereMeshNode, false);

    m_skeletonSphereMeshNode = m_scene->CreateEntity();

    if (auto* pModelComponent = m_skeletonSphereMeshNode->RegisterComponent<MModelComponent>())
    {
        std::shared_ptr<MSkeletonResource> pSkeleton = resourceSystem->CreateResource<MSkeletonResource>();
        pModelComponent->SetSkeletonResource(pSkeleton);
    }

    if (auto* meshComponent = m_skeletonSphereMeshNode->RegisterComponent<MRenderMeshComponent>())
    {
        std::shared_ptr<MMeshResource> pMeshResource = resourceSystem->CreateResource<MMeshResource>();
        pMeshResource->Load(MMeshResourceUtil::CreateSphere(MEMeshVertexType::Skeleton));
        meshComponent->Load(pMeshResource);
    }

    sceneSystem->SetVisible(m_skeletonSphereMeshNode, false);


    MEntity* pDirLight = m_scene->CreateEntity();
    pDirLight->SetName("DirLight");

    if (auto* pDirLightSceneComponent = pDirLight->RegisterComponent<MSceneComponent>())
    {
        Quaternion quat;
        quat.SetEulerAngle(Vector3(-45, 45, 0));
        pDirLightSceneComponent->SetRotation(Quaternion(quat));
    }

    if (auto* pDirLightComponent = pDirLight->RegisterComponent<MDirectionalLightComponent>())
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

void MaterialView::Input(MInputEvent* pEvent) { m_sceneTexture->GetViewport()->Input(pEvent); }
