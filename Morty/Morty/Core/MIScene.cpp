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
#include "MIViewport.h"

MIScene::MIScene()
	: MObject()
	, m_pRootNode(nullptr)
	, m_pSkyBox(nullptr)
	, m_pAttachedViewport(nullptr)
{
	
}

void MIScene::OnCreated()
{
	MObject::OnCreated();

	m_pSkyBox = m_pEngine->GetObjectManager()->CreateObject<MSkyBox>();
}

void MIScene::SetAttachedViewport(MIViewport* pViewport)
{
	m_pAttachedViewport = pViewport;
}

void MIScene::OnAddNode(MNode* pNode)
{
	if (MDirectionalLight* pDirLight = dynamic_cast<MDirectionalLight*>(pNode))
		m_vDirectionalLight.push_back(pDirLight);

	else if (MPointLight* pPotLight = dynamic_cast<MPointLight*>(pNode))
		m_vPointLight.push_back(pPotLight);
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

void MIScene::DrawNode(MIRenderer* pRenderer, MIViewport* pViewport, MNode* pNode)
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
				Matrix3 matNormal(worldTrans.Transposed().Inverse(), 3, 3);

				MStruct* pSpaceStruct = param.var.GetByType<MStruct>();
				pSpaceStruct->SetMember("MatWorld", worldTrans);
				pSpaceStruct->SetMember("MatCamProj", pViewport->GetCameraInverseProjection());

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

				param.var.GetByType<MStruct>()->SetMember("CameraWorldPos", pViewport->GetCamera()->GetPosition());
				break;
			}
		}

		pRenderer->SetUseMaterial(pMaterial);
		pRenderer->DrawMesh(pMeshIns->GetMesh());
	}

	for (MNode* pChild : pNode->GetChildren())
		DrawNode(pRenderer, pViewport, pChild);
}

void MIScene::DrawSkyBox(MIRenderer* pRenderer, MIViewport* pViewport)
{
	pRenderer->SetRasterizerType(MIRenderer::MERasterizerType::ESolid | MIRenderer::MERasterizerType::ECullNone);

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
					Matrix4 mat(Matrix4::IdentityMatrix);
					Vector3 camPos = pViewport->GetCamera()->GetPosition();
					mat.m[0][3] = camPos.x;
					mat.m[1][3] = camPos.y;
					mat.m[2][3] = camPos.z;
					pSpaceStruct->SetMember("MatWorld", mat);
					pSpaceStruct->SetMember("MatCamProj", pViewport->GetCameraInverseProjection());
					break;
				}
			}
			pRenderer->SetUseMaterial(pMaterial);
			pRenderer->DrawMesh(pMeshIns->GetMesh());
		}
		
	}
}

void MIScene::Render(MIRenderer* pRenderer, MIViewport* pViewport)
{
	//Draw
	DrawNode(pRenderer, pViewport, m_pRootNode);
	DrawSkyBox(pRenderer, pViewport);
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
