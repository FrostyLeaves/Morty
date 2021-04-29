#include "MaterialView.h"

#include "MNode.h"
#include "MScene.h"
#include "MEngine.h"
#include "MObject.h"
#include "MTexture.h"
#include "MMaterial.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "Model/MModelResource.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

#include "imgui.h"

#include "MSceneComponent.h"
#include "MModelComponent.h"
#include "MRenderableMeshComponent.h"
#include "MDirectionalLightComponent.h"

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

void MaterialView::SetMaterial(MMaterial* pMaterial)
{
	if (m_pMaterial == pMaterial)
		return;

	if (m_Resource.GetResource() == pMaterial)
		return;

	m_Resource.SetResource(pMaterial);
	m_pMaterial = pMaterial;

	if (!m_pMaterial)
	{
		m_pStaticSphereMeshNode->SetVisible(false);
		m_pSkeletonSphereMeshNode->SetVisible(false);
	}
	else if (m_pMaterial->GetShaderMacro()->GetInnerMacro(MGlobal::MATERIAL_MACRO_SKELETON_ENABLE).empty())
	{
		m_pStaticSphereMeshNode->SetVisible(true);
		m_pSkeletonSphereMeshNode->SetVisible(false);
	}
	else
	{
		m_pStaticSphereMeshNode->SetVisible(false);
		m_pSkeletonSphereMeshNode->SetVisible(true);
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

void MaterialView::UpdateTexture(MRenderCommand* pCommand)
{
	if (!m_bShowPreview)
		return;

	m_SceneTexture.UpdateTexture(pCommand);
}

void MaterialView::Render()
{
	if (m_pMaterial && m_bShowPreview)
	{
		if (void* pTexture = m_SceneTexture.GetTexture(m_pEngine->GetFrameIdx()))
		{
			ImTextureID texid = pTexture;
			float fImageSize = ImGui::GetContentRegionAvail().x;
			ImGui::SameLine(fImageSize * 0.25f);
			ImGui::Image(texid, ImVec2(fImageSize * 0.5f, fImageSize * 0.5f));
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
	m_SceneTexture.Initialize(pEngine);
	m_SceneTexture.SetSize(Vector2(512, 512));

	m_pEngine = pEngine;

	MScene* pScene = m_SceneTexture.GetScene();
	MNode* pRootNode = pScene->GetRootNode();
	if (nullptr == pRootNode)
	{
		pRootNode = pEngine->GetObjectManager()->CreateObject<MNode>();
		pScene->SetRootNode(pRootNode);
	}

	MNode* pCameraNode = m_SceneTexture.GetViewport()->GetCamera();
	if (MSceneComponent* pCameraSceneComponent = pCameraNode->GetComponent<MSceneComponent>())
	{
		pCameraSceneComponent->SetPosition(Vector3(0, 0, -5));
	}

	m_pStaticSphereMeshNode = m_pEngine->GetObjectManager()->CreateObject<MNode>();
	pRootNode->AddNode(m_pStaticSphereMeshNode);

	if (MSceneComponent* pSceneComponent = m_pStaticSphereMeshNode->RegisterComponent<MSceneComponent>())
	{

	}

	if (MRenderableMeshComponent* pMeshComponent = m_pStaticSphereMeshNode->RegisterComponent<MRenderableMeshComponent>())
	{
		if (MResource* pResource = m_pEngine->GetResourceManager()->LoadResource("./Model/Sphere/GeoSphere001.mesh"))
			pMeshComponent->Load(pResource);
	}

	m_pStaticSphereMeshNode->SetVisible(false);



	m_pSkeletonSphereMeshNode = m_pEngine->GetObjectManager()->CreateObject<MNode>();
	pRootNode->AddNode(m_pSkeletonSphereMeshNode);
	
	if (MSceneComponent* pSceneComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MSceneComponent>())
	{

	}

	if (MModelComponent* pModelComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MModelComponent>())
	{
		MSkeletonResource* pSkeleton = m_pEngine->GetResourceManager()->CreateResource<MSkeletonResource>();
		pModelComponent->SetSkeletonResource(pSkeleton);
	}

	if (MRenderableMeshComponent* pMeshComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MRenderableMeshComponent>())
	{
		if (MResource* pResource = m_pEngine->GetResourceManager()->LoadResource("./Model/Sphere/GeoSphere001_anim.mesh"))
			pMeshComponent->Load(pResource);
	}

	m_pSkeletonSphereMeshNode->SetVisible(false);





// 	unsigned int nSize = pResource->GetMesh()->GetVerticesLength();
// 	uint32_t nIdxSize = pResource->GetMesh()->GetIndicesLength();
// 	MVertex* pVertex = (MVertex*)pResource->GetMesh()->GetVertices();
// 	uint32_t* pIdxxx = pResource->GetMesh()->GetIndices();
// 
// 	MMesh<MVertexWithBones>* pNewVertex = new MMesh<MVertexWithBones>();
// 	pNewVertex->CreateVertices(nSize);
// 	pNewVertex->CreateIndices(pResource->GetMesh()->GetIndicesLength(), 1);
// 
// 	MVertexWithBones* pBoneVertex = pNewVertex->GetVertices();
// 	uint32_t* pIndices = pNewVertex->GetIndices();
// 
// 	for (int i = 0; i < nSize; ++i)
// 	{
// 		memset(&(pBoneVertex[i]), 0, sizeof(MVertexWithBones));
// 		pBoneVertex[i].position = pVertex[i].position;
// 		pBoneVertex[i].bitangent = pVertex[i].bitangent;
// 		pBoneVertex[i].normal = pVertex[i].normal;
// 		pBoneVertex[i].tangent = pVertex[i].tangent;
// 		pBoneVertex[i].texCoords = pVertex[i].texCoords;
// 	}
// 
// 	memcpy(pIndices, pIdxxx, sizeof(uint32_t) * nIdxSize);
// 	
// 
// 	pResource->m_pMesh = pNewVertex;
// 	pResource->m_eVertexType = MMeshResource::Skeleton;
// 
// 
// 	pResource->SaveTo("./Model/Sphere/GeoSphere001_anim.mesh");

 	MNode* pDirLight = m_pEngine->GetObjectManager()->CreateObject<MNode>();
 	pDirLight->SetName("DirLight");
 	pRootNode->AddNode(pDirLight);

	if (MSceneComponent* pDirLightSceneComponent = pDirLight->RegisterComponent<MSceneComponent>())
	{
		Quaternion quat;
		quat.SetEulerAngle(Vector3(-45, 45, 0));
		pDirLightSceneComponent->SetRotation(Quaternion(quat));
	}

	if (MDirectionalLightComponent* pDirLightComponent = pDirLight->RegisterComponent<MDirectionalLightComponent>())
	{
		pDirLightComponent->SetLightIntensity(100.0f);
	}
	
}

void MaterialView::Release()
{
	SetMaterial(nullptr);

	m_SceneTexture.Release();


	m_pSkeletonSphereMeshNode->DeleteLater();
	m_pSkeletonSphereMeshNode = nullptr;

	m_pStaticSphereMeshNode->DeleteLater();
	m_pStaticSphereMeshNode = nullptr;
}

void MaterialView::Input(MInputEvent* pEvent)
{
	m_SceneTexture.GetViewport()->Input(pEvent);
}
