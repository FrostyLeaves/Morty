#include "MaterialView.h"

#include "MScene.h"
#include "MEntity.h"
#include "MEngine.h"
#include "MObject.h"
#include "MTexture.h"
#include "MMaterial.h"
#include "MViewport.h"
#include "MMaterialResource.h"

#include "imgui.h"

#include "MSceneSystem.h"
#include "MObjectSystem.h"
#include "MResourceSystem.h"

#include "MSceneComponent.h"
#include "MModelComponent.h"
#include "MRenderableMeshComponent.h"
#include "MDirectionalLightComponent.h"

#include "MSkeletonResource.h"

#include "MForwardRenderProgram.h"

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

	MSceneSystem* pSceneSystem = m_pEngine->FindSystem<MSceneSystem>();

	m_Resource.SetResource(pMaterial);
	m_pMaterial = pMaterial;

	if (!m_pMaterial)
	{
		pSceneSystem->SetVisible(m_pStaticSphereMeshNode, false);
		pSceneSystem->SetVisible(m_pSkeletonSphereMeshNode, false);
	}
	else if (m_pMaterial->GetShaderMacro()->GetInnerMacro(MRenderGlobal::SHADER_SKELETON_ENABLE).empty())
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

void MaterialView::UpdateTexture(MIRenderCommand* pCommand)
{

}

void MaterialView::Render()
{
// 	if (m_pMaterial && m_bShowPreview)
// 	{
// 		if (void* pTexture = m_SceneTexture.GetTexture())
// 		{
// 			ImTextureID texid = pTexture;
// 			float fImageSize = ImGui::GetContentRegionAvail().x;
// 			ImGui::SameLine(fImageSize * 0.25f);
// 			ImGui::Image(texid, ImVec2(fImageSize * 0.5f, fImageSize * 0.5f));
// 		}
// 	}
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

	m_SceneTexture.Initialize(pEngine, MForwardRenderProgram::GetClassTypeName(), 1);
	m_SceneTexture.SetSize(Vector2(512, 512));


	MScene* pScene = m_SceneTexture.GetScene();
	if (!pScene)
		return;
	
	MEntity* pRootNode = pScene->CreateEntity();

	if (MEntity* pCameraNode = m_SceneTexture.GetViewport()->GetCamera())
	{
		if (MSceneComponent* pCameraSceneComponent = pCameraNode->GetComponent<MSceneComponent>())
		{
			pCameraSceneComponent->SetPosition(Vector3(0, 0, -5));
		}
	}

	m_pStaticSphereMeshNode = pScene->CreateEntity();

	if (MSceneComponent* pSceneComponent = m_pStaticSphereMeshNode->RegisterComponent<MSceneComponent>())
	{

	}

	if (MRenderableMeshComponent* pMeshComponent = m_pStaticSphereMeshNode->RegisterComponent<MRenderableMeshComponent>())
	{
		if (MResource* pResource = pResourceSystem->LoadResource("./Model/Sphere/GeoSphere001.mesh"))
			pMeshComponent->Load(pResource);
	}

	pSceneSystem->SetVisible(m_pStaticSphereMeshNode, false);



	m_pSkeletonSphereMeshNode = pScene->CreateEntity();
	
	if (MSceneComponent* pSceneComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MSceneComponent>())
	{

	}

	if (MModelComponent* pModelComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MModelComponent>())
	{
		MSkeletonResource* pSkeleton = pResourceSystem->CreateResource<MSkeletonResource>();
		pModelComponent->SetSkeletonResource(pSkeleton);
	}

	if (MRenderableMeshComponent* pMeshComponent = m_pSkeletonSphereMeshNode->RegisterComponent<MRenderableMeshComponent>())
	{
		if (MResource* pResource = pResourceSystem->LoadResource("./Model/Sphere/GeoSphere001_anim.mesh"))
			pMeshComponent->Load(pResource);
	}

	pSceneSystem->SetVisible(m_pSkeletonSphereMeshNode, false);





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
		pDirLightComponent->SetLightIntensity(100.0f);
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
