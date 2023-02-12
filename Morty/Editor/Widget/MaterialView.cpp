#include "MaterialView.h"

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Object/MObject.h"
#include "Basic/MTexture.h"
#include "Material/MMaterial.h"
#include "Basic/MViewport.h"
#include "Resource/MMaterialResource.h"

#include "imgui.h"

#include "System/MSceneSystem.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include "Component/MSceneComponent.h"
#include "Component/MModelComponent.h"
#include "Component/MRenderableMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"

#include "Resource/MSkeletonResource.h"

#include "RenderProgram/MIRenderProgram.h"

#include "Main/MainEditor.h"

MaterialView::MaterialView()
	: IBaseView()
	, m_Resource()
	, m_pMaterial(nullptr)
	, m_bShowPreview(true)
	, m_pStaticSphereMeshNode(nullptr)
	, m_pSkeletonSphereMeshNode(nullptr)
{

}

MaterialView::~MaterialView()
{
}

void MaterialView::SetMaterial(std::shared_ptr<MMaterial> pMaterial)
{
	if (m_pMaterial == pMaterial)
		return;

	if (m_Resource.GetResource() == pMaterial)
		return;

	MSceneSystem* pSceneSystem = m_pEngine->FindSystem<MSceneSystem>();

	m_Resource.SetResource(pMaterial);
	m_pMaterial = pMaterial;

	if (!m_pMaterial)
	{
		pSceneSystem->SetVisible(m_pStaticSphereMeshNode, false);
		pSceneSystem->SetVisible(m_pSkeletonSphereMeshNode, false);
	}
	else if (m_pMaterial->GetShaderMacro().GetInnerMacro(MRenderGlobal::SHADER_SKELETON_ENABLE).empty())
	{
		pSceneSystem->SetVisible(m_pStaticSphereMeshNode, true);
		pSceneSystem->SetVisible(m_pSkeletonSphereMeshNode, false);
	}
	else
	{
		pSceneSystem->SetVisible(m_pStaticSphereMeshNode, false);
		pSceneSystem->SetVisible(m_pSkeletonSphereMeshNode, true);
	}

	if (MRenderableMeshComponent* pMeshComponent = m_pStaticSphereMeshNode->GetComponent<MRenderableMeshComponent>())
	{
		pMeshComponent->SetMaterial(pMaterial);
	}
	if (MRenderableMeshComponent* pMeshComponent = m_pSkeletonSphereMeshNode->GetComponent<MRenderableMeshComponent>())
	{
		pMeshComponent->SetMaterial(pMaterial);
	}
}

void MaterialView::Render()
{
 	if (m_pMaterial && m_bShowPreview)
 	{
 		if (std::shared_ptr<MTexture> pTexture = m_SceneTexture.GetTexture(0))
 		{
 			float fImageSize = ImGui::GetContentRegionAvail().x;
 			ImGui::SameLine(fImageSize * 0.25f);
 			ImGui::Image({ pTexture, 0 }, ImVec2(fImageSize * 0.5f, fImageSize * 0.5f));
 		}
 	}
	if (m_pMaterial)
	{

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();

		m_propertyBase.EditMMaterial(m_pMaterial);

		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopStyleVar();
	}
}

void MaterialView::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	MSceneSystem* pSceneSystem = pEngine->FindSystem<MSceneSystem>();
	MObjectSystem* pObjectSystem = pEngine->FindSystem<MObjectSystem>();
	MResourceSystem* pResourceSystem = pEngine->FindSystem<MResourceSystem>();

	m_SceneTexture.Initialize(pEngine, MainEditor::GetRenderProgramName(), 1);
	m_SceneTexture.SetSize(Vector2(512, 512));
	m_SceneTexture.SetBackColor(MColor(1.0f, 0.0f, 0.0f, 1.0f));


	MScene* pScene = m_SceneTexture.GetScene();
	if (!pScene)
		return;
	
	MEntity* pRootNode = pScene->CreateEntity();

	if (MEntity* pCameraNode = m_SceneTexture.GetViewport()->GetCamera())
	{
		if (MSceneComponent* pCameraSceneComponent = pCameraNode->GetComponent<MSceneComponent>())
		{
			pCameraSceneComponent->SetPosition(Vector3(0, 0, -5));
			pCameraSceneComponent->SetRotation(UnitQuaternion);
		}
	}

	m_pStaticSphereMeshNode = pScene->CreateEntity();

	if (MSceneComponent* pSceneComponent = m_pStaticSphereMeshNode->RegisterComponent<MSceneComponent>())
	{

	}

	if (MRenderableMeshComponent* pMeshComponent = m_pStaticSphereMeshNode->RegisterComponent<MRenderableMeshComponent>())
	{
		std::shared_ptr<MMeshResource> pMeshResource = pResourceSystem->CreateResource<MMeshResource>();
		pMeshResource->LoadAsSphere();
		pMeshComponent->Load(pMeshResource);
	}

	pSceneSystem->SetVisible(m_pStaticSphereMeshNode, false);



	m_pSkeletonSphereMeshNode = pScene->CreateEntity();
	
	if (MSceneComponent* pSceneComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MSceneComponent>())
	{

	}

	if (MModelComponent* pModelComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MModelComponent>())
	{
		std::shared_ptr<MSkeletonResource> pSkeleton = pResourceSystem->CreateResource<MSkeletonResource>();
		pModelComponent->SetSkeletonResource(pSkeleton);
	}

	if (MRenderableMeshComponent* pMeshComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MRenderableMeshComponent>())
	{
		std::shared_ptr<MMeshResource> pMeshResource = pResourceSystem->CreateResource<MMeshResource>();
		pMeshResource->LoadAsSphere(MMeshResource::MEMeshVertexType::Skeleton);
		pMeshComponent->Load(pMeshResource);
	}

	pSceneSystem->SetVisible(m_pSkeletonSphereMeshNode, false);


 	MEntity* pDirLight = pScene->CreateEntity();
 	pDirLight->SetName("DirLight");

	if (MSceneComponent* pDirLightSceneComponent = pDirLight->RegisterComponent<MSceneComponent>())
	{
		Quaternion quat;
		quat.SetEulerAngle(Vector3(-45, 45, 0));
		pDirLightSceneComponent->SetRotation(Quaternion(quat));
	}

	if (MDirectionalLightComponent* pDirLightComponent = pDirLight->RegisterComponent<MDirectionalLightComponent>())
	{
		pDirLightComponent->SetLightIntensity(1.0f);
	}
	
}

void MaterialView::Release()
{
	SetMaterial(nullptr);

	MScene* pScene = m_SceneTexture.GetScene();
	pScene->DeleteEntity(m_pSkeletonSphereMeshNode);
	m_pSkeletonSphereMeshNode = nullptr;
	pScene->DeleteEntity(m_pStaticSphereMeshNode);
	m_pStaticSphereMeshNode = nullptr;

	m_SceneTexture.Release();
}

void MaterialView::Input(MInputEvent* pEvent)
{
	m_SceneTexture.GetViewport()->Input(pEvent);
}
