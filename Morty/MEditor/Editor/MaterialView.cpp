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
#include "MTextureRenderTarget.h"
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

	m_bShowPreview = m_pMaterial && m_pMaterial->GetShaderMacro()->GetInnerMacro(MATERIAL_MACRO_SKELETON_ENABLE).empty();

	m_pMeshInstance->SetMaterial(pMaterial);
}

void MaterialView::UpdateMaterialTexture()
{
	if (!m_bShowPreview)
		return;

	m_SceneTexture.UpdateTexture();
}

void MaterialView::Render()
{
	if (m_pMaterial && m_bShowPreview)
	{
		if (void* pTexture = m_SceneTexture.GetTexture(m_pEngine->GetRenderer()->GetFrameIndex()))
		{
			ImTextureID texid;
			texid.pTexture = pTexture;
			texid.nType = 0;
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

	m_pEngine = pEngine;

	MScene* pScene = m_SceneTexture.GetScene();
	MNode* pRootNode = pScene->GetRootNode();
	if (nullptr == pRootNode)
	{
		pRootNode = pEngine->GetObjectManager()->CreateObject<M3DNode>();
		pScene->SetRootNode(pRootNode);
	}

	MCamera* pCamera = m_SceneTexture.GetViewport()->GetCamera();
	pCamera->SetPosition(Vector3(0, 0, -20));

	MResource* pResource = m_pEngine->GetResourceManager()->LoadResource("./Model/Sphere/GeoSphere001.mesh");

	m_pMeshInstance = m_pEngine->GetObjectManager()->CreateObject<MStaticMeshInstance>();
	m_pMeshInstance->Load(pResource);
	pRootNode->AddNode(m_pMeshInstance);

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
}

void MaterialView::Input(MInputEvent* pEvent)
{
	m_SceneTexture.GetViewport()->Input(pEvent);
}
