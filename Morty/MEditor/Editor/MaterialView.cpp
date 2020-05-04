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
#include "Model/MModelInstance.h"
#include "MResourceManager.h"
#include "Material/MMaterialResource.h"

#include "imgui.h"

MaterialView::MaterialView()
	: IBaseView()
	, m_Resource()
	, m_pMaterial(nullptr)
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

	m_pMeshInstance->SetMaterial(pMaterial);
}

void MaterialView::UpdateMaterialTexture()
{
	m_SceneTexture.UpdateTexture();
}

void MaterialView::Render()
{
	if (m_pMaterial)
	{
		if (ImTextureID texid = m_SceneTexture.GetTexture())
		{
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

	MCamera* pCamera = m_SceneTexture.GetViewport()->GetCamera();
	pCamera->SetPosition(Vector3(0, 0, -20));

	MResource* pResource = m_pEngine->GetResourceManager()->LoadResource("./Model/Sphere.fbx");

	MModelInstance* pModel = m_pEngine->GetObjectManager()->CreateObject<MModelInstance>();
	pModel->Load(pResource);
	pRootNode->AddNode(pModel);

	m_pMeshInstance = pModel->GetFixedChildren()[0]->DynamicCast<MIMeshInstance>();

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
