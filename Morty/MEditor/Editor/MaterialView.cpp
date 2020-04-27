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

#include "imgui.h"

MaterialView::MaterialView()
	: IBaseView()
	, m_pResource(nullptr)
	, m_pMaterial(nullptr)
{

}

MaterialView::~MaterialView()
{
	if (m_pResource)
	{
		SetMaterial(nullptr);
	}
	m_pMaterial = nullptr;
}

void MaterialView::SetMaterial(MMaterial* pMaterial)
{
	if (m_pMaterial == pMaterial)
		return;

	if (m_pResource)
	{
		if (m_pResource->GetResource() == pMaterial)
			return;
		else
		{
			delete m_pResource;
			m_pResource = nullptr;
			m_pMaterial = nullptr;
		}
	}

	m_pResource = new MResourceHolder(pMaterial);
	m_pMaterial = pMaterial;

	m_pMeshInstance->SetMaterial(pMaterial);

}

void MaterialView::UpdateMaterialTexture()
{
	m_pEngine->GetRenderer()->Render(m_pTextureRenderTarget);
}

void MaterialView::Render()
{
	if (m_pMaterial)
	{
		if (m_pTextureRenderTarget)
		{
			if (m_pTextureRenderTarget)
			{
				if (m_pTextureRenderTarget->m_pBackTexture)
				{
					if (MTextureBuffer* pBuffer = m_pTextureRenderTarget->m_pBackTexture->GetBuffer())
					{
						float fImageSize = ImGui::GetContentRegionAvail().x;
						ImGui::SameLine(fImageSize * 0.25f);
						ImGui::Image(pBuffer->m_pShaderResourceView, ImVec2(fImageSize * 0.5f, fImageSize * 0.5f));
					}
				}
			}
		}


	}
	if (m_pMaterial)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();

		unsigned int unID = 0;

		m_propertyBase.EditMMaterial(m_pMaterial);

		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopStyleVar();
	}
}

void MaterialView::Initialize(MEngine* pEngine)
{
	m_pEngine = pEngine;

	//Setup Render
	m_pScene = m_pEngine->GetObjectManager()->CreateObject<MScene>();

	m_pRenderViewport = pEngine->GetObjectManager()->CreateObject<MViewport>();
	m_pRenderViewport->SetScene(m_pScene);
	m_pRenderViewport->SetSize(Vector2(256, 256));
	m_pTextureRenderTarget = MTextureRenderTarget::CreateForTexture(m_pEngine->GetDevice(), MTextureRenderTarget::ERenderBack | MTextureRenderTarget::ERenderDepth, 256, 256);
	m_pTextureRenderTarget->m_backgroundColor = MColor(0, 0, 0, 1);
	m_pTextureRenderTarget->m_funcRenderFunction = [this](MIRenderer* pRenderer)
	{
		m_pRenderViewport->Render(pRenderer);
	};

	M3DNode* pRootNode = m_pEngine->GetObjectManager()->CreateObject<M3DNode>();

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

	m_pScene->SetRootNode(pRootNode);

	MCamera* pCamera = m_pRenderViewport->GetCamera();
	pCamera->SetPosition(Vector3(0, 0, -20));
}

void MaterialView::Release()
{
	//TODO Release RenderTarget and Viewport
	MObjectManager* pObjManager = m_pEngine->GetObjectManager();

	m_pScene->DeleteLater();
	m_pScene = nullptr;

	m_pRenderViewport->DeleteLater();
	m_pRenderViewport = nullptr;
}

void MaterialView::Input(MInputEvent* pEvent)
{
	m_pRenderViewport->Input(pEvent);
}
