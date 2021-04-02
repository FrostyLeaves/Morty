#include "MaterialView.h"
#include "MScene.h"
#include "MCamera.h"
#include "MEngine.h"
#include "MObject.h"
#include "MTexture.h"
#include "MMaterial.h"
#include "MViewport.h"
#include "MIRenderer.h"
#include "Light/MDirectionalLight.h"
#include "Model/MIMeshInstance.h"
#include "Model/MModelInstance.h"
#include "Model/MModelResource.h"
#include "Model/MStaticMeshInstance.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

#include "imgui.h"

MaterialView::MaterialView()
	: IBaseView()
	, m_Resource()
	, m_pMaterial(nullptr)
	, m_bShowPreview(true)
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
		m_pMeshInstance->SetVisible(false);
		m_pSkeletonMeshInstance->SetVisible(false);
	}
	else if (m_pMaterial->GetShaderMacro()->GetInnerMacro(MGlobal::MATERIAL_MACRO_SKELETON_ENABLE).empty())
	{
		m_pMeshInstance->SetVisible(true);
		m_pSkeletonMeshInstance->SetVisible(false);
	}
	else
	{
		m_pMeshInstance->SetVisible(false);
		m_pSkeletonMeshInstance->SetVisible(true);
	}

	m_pMeshInstance->SetMaterial(pMaterial);
	m_pSkeletonMeshInstance->SetMaterial(pMaterial);
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
		pRootNode = pEngine->GetObjectManager()->CreateObject<M3DNode>();
		pScene->SetRootNode(pRootNode);
	}

	MCamera* pCamera = m_SceneTexture.GetViewport()->GetCamera();
	pCamera->SetPosition(Vector3(0, 0, -5));

	m_pMeshInstance = m_pEngine->GetObjectManager()->CreateObject<MStaticMeshInstance>();
	if(MResource* pResource = m_pEngine->GetResourceManager()->LoadResource("./Model/Sphere/GeoSphere001.mesh"))
		m_pMeshInstance->Load(pResource);
	pRootNode->AddNode(m_pMeshInstance);
	m_pMeshInstance->SetVisible(false);


	MSkeletonResource* pSkeleton = m_pEngine->GetResourceManager()->CreateResource<MSkeletonResource>();

	MModelInstance* pModelInstance = m_pEngine->GetObjectManager()->CreateObject<MModelInstance>();
	pModelInstance->SetSkeletonTemplate(pSkeleton);
	pRootNode->AddNode(pModelInstance);

	m_pSkeletonMeshInstance = m_pEngine->GetObjectManager()->CreateObject<MStaticMeshInstance>();
	if (MResource* pResource = m_pEngine->GetResourceManager()->LoadResource("./Model/Sphere/GeoSphere001_anim.mesh"))
		m_pSkeletonMeshInstance->Load(pResource);
	pModelInstance->AddNode(m_pSkeletonMeshInstance);
	m_pMeshInstance->SetVisible(false);





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

 	MDirectionalLight* pDirLight = m_pEngine->GetObjectManager()->CreateObject<MDirectionalLight>();
 	pDirLight->SetName("DirLight");
 	pRootNode->AddNode(pDirLight);

	Quaternion quat;
	quat.SetEulerAngle(Vector3(-45, 45, 0));
	pDirLight->SetRotation(Quaternion(quat));
}

void MaterialView::Release()
{
	SetMaterial(nullptr);

	m_SceneTexture.Release();


	m_pMeshInstance->DeleteLater();
	m_pMeshInstance = nullptr;

	m_pSkeletonMeshInstance->DeleteLater();
	m_pSkeletonMeshInstance = nullptr;
}

void MaterialView::Input(MInputEvent* pEvent)
{
	m_SceneTexture.GetViewport()->Input(pEvent);
}
