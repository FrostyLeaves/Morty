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
#include "Component/MRenderMeshComponent.h"
#include "Component/MDirectionalLightComponent.h"

#include "Resource/MSkeletonResource.h"

#include "RenderProgram/MIRenderProgram.h"

#include "Main/MainEditor.h"
#include "Resource/MMeshResourceUtil.h"
#include "Utility/SelectionEntityManager.h"

MaterialView::MaterialView()
	: BaseWidget()
{
	m_strViewName = "Material";
}

void MaterialView::SetMaterial(std::shared_ptr<MMaterialResource> pMaterial)
{
	if (m_pMaterial == pMaterial)
		return;

	MSceneSystem* pSceneSystem = GetEngine()->FindSystem<MSceneSystem>();

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

		if (MRenderMeshComponent* pMeshComponent = m_pStaticSphereMeshNode->GetComponent<MRenderMeshComponent>())
		{
			pMeshComponent->SetMaterial(pMaterial);
		}
	}
	else
	{
		pSceneSystem->SetVisible(m_pStaticSphereMeshNode, false);
		pSceneSystem->SetVisible(m_pSkeletonSphereMeshNode, true);

		if (MRenderMeshComponent* pMeshComponent = m_pSkeletonSphereMeshNode->GetComponent<MRenderMeshComponent>())
		{
			pMeshComponent->SetMaterial(pMaterial);
		}
	}
}

void MaterialView::Render()
{
	if (MEntity* pEntity = SelectionEntityManager::GetInstance()->GetSelectedEntity())
	{
		if (MRenderMeshComponent* pMeshComponent = pEntity->GetComponent<MRenderMeshComponent>())
		{
			SetMaterial(pMeshComponent->GetMaterialResource());
		}
	}
	
 	if (m_pMaterial && m_bShowPreview)
 	{
 		if (std::shared_ptr<MTexture> pTexture = m_pSceneTexture->GetTexture(0))
 		{
 			float fImageSize = ImGui::GetContentRegionAvail().x;
 			ImGui::SameLine(fImageSize * 0.25f);
 			ImGui::Image({ pTexture, intptr_t(pTexture.get()), 0 }, ImVec2(fImageSize * 0.5f, fImageSize * 0.5f));
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

void MaterialView::Initialize(MainEditor* pMainEditor)
{
	BaseWidget::Initialize(pMainEditor);

	MSceneSystem* pSceneSystem = GetEngine()->FindSystem<MSceneSystem>();
	MObjectSystem* pObjectSystem = GetEngine()->FindSystem<MObjectSystem>();
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	m_pScene = pObjectSystem->CreateObject<MScene>();

	m_pSceneTexture = GetMainEditor()->CreateSceneViewer(m_pScene);
	m_pSceneTexture->SetRect(Vector2i(0, 0), Vector2i(512, 512));

	if (MEntity* pCameraNode = m_pSceneTexture->GetViewport()->GetCamera())
	{
		if (MSceneComponent* pCameraSceneComponent = pCameraNode->GetComponent<MSceneComponent>())
		{
			pCameraSceneComponent->SetPosition(Vector3(0, 0, -5));
			pCameraSceneComponent->SetRotation(UnitQuaternion);
		}
	}

	m_pStaticSphereMeshNode = m_pScene->CreateEntity();

	if (MSceneComponent* pSceneComponent = m_pStaticSphereMeshNode->RegisterComponent<MSceneComponent>())
	{

	}

	if (MRenderMeshComponent* pMeshComponent = m_pStaticSphereMeshNode->RegisterComponent<MRenderMeshComponent>())
	{
		std::shared_ptr<MMeshResource> pMeshResource = pResourceSystem->CreateResource<MMeshResource>();
	    pMeshResource->Load(MMeshResourceUtil::CreateSphere());
		pMeshComponent->Load(pMeshResource);
	}

	pSceneSystem->SetVisible(m_pStaticSphereMeshNode, false);



	m_pSkeletonSphereMeshNode = m_pScene->CreateEntity();
	
	if (MSceneComponent* pSceneComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MSceneComponent>())
	{

	}

	if (MModelComponent* pModelComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MModelComponent>())
	{
		std::shared_ptr<MSkeletonResource> pSkeleton = pResourceSystem->CreateResource<MSkeletonResource>();
		pModelComponent->SetSkeletonResource(pSkeleton);
	}

	if (MRenderMeshComponent* pMeshComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MRenderMeshComponent>())
	{
		std::shared_ptr<MMeshResource> pMeshResource = pResourceSystem->CreateResource<MMeshResource>();
		pMeshResource->Load(MMeshResourceUtil::CreateSphere(MEMeshVertexType::Skeleton));
		pMeshComponent->Load(pMeshResource);
	}

	pSceneSystem->SetVisible(m_pSkeletonSphereMeshNode, false);


 	MEntity* pDirLight = m_pScene->CreateEntity();
 	pDirLight->SetName("DirLight");

	if (MSceneComponent* pDirLightSceneComponent = pDirLight->RegisterComponent<MSceneComponent>())
	{
		Quaternion quat;
		quat.SetEulerAngle(Vector3(-45, 45, 0));
		pDirLightSceneComponent->SetRotation(Quaternion(quat));
	}

	if (MDirectionalLightComponent* pDirLightComponent = pDirLight->RegisterComponent<MDirectionalLightComponent>())
	{
		pDirLightComponent->SetLightIntensity(10.0f);
	}
	
}

void MaterialView::Release()
{
	SetMaterial(nullptr);
	
	m_pScene->DeleteEntity(m_pSkeletonSphereMeshNode);
	m_pSkeletonSphereMeshNode = nullptr;
	m_pScene->DeleteEntity(m_pStaticSphereMeshNode);
	m_pStaticSphereMeshNode = nullptr;
	m_pScene->DeleteLater();
	m_pScene = nullptr;

	GetMainEditor()->DestroySceneViewer(m_pSceneTexture);
	m_pSceneTexture = nullptr;
}

void MaterialView::Input(MInputEvent* pEvent)
{
	m_pSceneTexture->GetViewport()->Input(pEvent);
}
