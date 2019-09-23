#include "MIScene.h"
#include "MEngine.h"
#include "MCamera.h"
#include "MSkyBox.h"

#include "MDirectionalLight.h"
#include "MPointLight.h"

#include "MMeshInstance.h"
#include "MVertex.h"
#include "MMaterial.h"
#include "MIRenderer.h"
#include "MIRenderView.h"

MIScene::MIScene()
	: MObject()
	, m_pRootNode(nullptr)
	, m_pUsingCamera(nullptr)
	, m_pDefaultCamera(nullptr)
	, m_pSkyBox(nullptr)
	, m_m4CameraInvProj(IdentityMatrix)
{
	
}

void MIScene::OnCreated()
{
	//Init Default Camera.
	m_pDefaultCamera = m_pEngine->GetObjectManager()->CreateObject<MCamera>();

	m_pSkyBox = m_pEngine->GetObjectManager()->CreateObject<MSkyBox>();
}

void MIScene::OnAddNode(MNode* pNode)
{
	if (MDirectionalLight* pDirLight = dynamic_cast<MDirectionalLight*>(pNode))
		m_vDirectionalLight.push_back(pDirLight);

	else if (MPointLight* pPotLight = dynamic_cast<MPointLight*>(pNode))
		m_vPointLight.push_back(pPotLight);

	else if (MCamera* pCamera = dynamic_cast<MCamera*>(pNode))
	{
		if (nullptr == m_pUsingCamera)
			m_pUsingCamera = pCamera;
	}
}

void MIScene::OnRemoveNode(MNode* pNode)
{
	if (MDirectionalLight* pDirLight = dynamic_cast<MDirectionalLight*>(pNode))
	{
		std::vector<MDirectionalLight*>::iterator iter = std::find(m_vDirectionalLight.begin(), m_vDirectionalLight.end(), pDirLight);
		if(m_vDirectionalLight.end() != iter)
			m_vDirectionalLight.erase(iter);
	}

	else if (MPointLight* pPotLight = dynamic_cast<MPointLight*>(pNode))
	{
		std::vector<MPointLight*>::iterator iter = std::find(m_vPointLight.begin(), m_vPointLight.end(), pPotLight);
		if (m_vPointLight.end() != iter)
			m_vPointLight.erase(iter);
	}
}

void MIScene::DrawNode(MIRenderer* pRenderer, MNode* pNode)
{
	if (nullptr == pNode)
		return;

	if (false == pNode->GetVisible())
		return;

	MMeshInstance* pMeshIns = dynamic_cast<MMeshInstance*>(pNode);
	if (pMeshIns)
	{
		MMaterial* pMaterial = pMeshIns->GetMaterial();


		std::vector<MShaderParam>& vVtxParams = pMaterial->GetVertexShaderParams();
		for (MShaderParam& param : vVtxParams)
		{
			if (param.strName == "cbSpace")
			{
				Matrix4 worldTrans = pMeshIns->GetWorldTransform();

				//Transposed and Inverse. but hlsl has been transporsed.
				Matrix3 matNormal(worldTrans.Inverse(), 3, 3);

				MStruct* pSpaceStruct = param.var.GetByType<MStruct>();
				pSpaceStruct->SetMember("MatWorld", worldTrans.Transposed());
				pSpaceStruct->SetMember("MatCamProj", m_m4CameraInvProj.Transposed());

				pSpaceStruct->SetMember("MatNormal", matNormal);
				break;
			}
		}

		std::vector<MShaderParam>& vPixParams = pMaterial->GetPixelShaderParams();
		for (MShaderParam& param : vPixParams)
		{
			if (param.strName == "cbLight")
			{
				param.var.GetByType<MStruct>()->SetMember("AmbientLightColor", Vector3(1, 1, 1));

				param.var.GetByType<MStruct>()->SetMember("DiffuseLightPos", Vector3(100, 100, 200));
				param.var.GetByType<MStruct>()->SetMember("DiffuseLightColor", Vector3(1, 1, 1));

				param.var.GetByType<MStruct>()->SetMember("CameraWorldPos", GetCamera()->GetPosition());
				break;
			}
		}

		pRenderer->SetUseMaterial(pMaterial);
		pRenderer->DrawMesh(pMeshIns->GetMesh());
	}

	for (MNode* pChild : pNode->GetChildren())
		DrawNode(pRenderer, pChild);
}

void MIScene::DrawSkyBox(MIRenderer* pRenderer)
{
	if (m_pSkyBox)
	{
		if (MMeshInstance* pMeshIns = m_pSkyBox->GetMeshInstance())
		{
			MMaterial* pMaterial = pMeshIns->GetMaterial();
			std::vector<MShaderParam>& vVtxParams = pMaterial->GetVertexShaderParams();
			for (MShaderParam& param : vVtxParams)
			{
				if (param.strName == "cbSpace")
				{
					MStruct* pSpaceStruct = param.var.GetByType<MStruct>();
					Matrix4 mat(IdentityMatrix);
					Vector3 camPos = GetCamera()->GetPosition();
					mat.SetTranslation(camPos.x, camPos.y, camPos.z);
					pSpaceStruct->SetMember("MatWorld", mat.Transposed());
					pSpaceStruct->SetMember("MatCamProj", m_m4CameraInvProj.Transposed());
					break;
				}
			}
			pRenderer->SetUseMaterial(pMaterial);
			pRenderer->DrawMesh(pMeshIns->GetMesh());
		}
		
	}
}

void MIScene::Render(MIRenderer* pRenderer, MIRenderView* pRenderView)
{
	//Get Camera Inverse.
	MCamera* pCamera = GetCamera();
	Matrix4 projMat = Matrix4::MatrixPerspectiveFovLH(45, (float)pRenderView->GetViewWidth() / pRenderView->GetViewHeight(), pCamera->GetZNear(), pCamera->GetZFar());
	m_m4CameraInvProj = GetCamera()->GetWorldTransform().Inverse() * projMat;

	//Draw
	DrawNode(pRenderer, m_pRootNode);


	DrawSkyBox(pRenderer);
}

MIScene::~MIScene()
{

}

void MIScene::SetRootNode(MNode* pRootNode)
{
	m_pRootNode = pRootNode;
	if (pRootNode)
	{
		pRootNode->SetAttachedScene(this);
	}
}

MCamera* MIScene::GetCamera()
{
	if (m_pUsingCamera)
		return m_pUsingCamera;
	return m_pDefaultCamera;
}
