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

void MIScene::OnNodeEnter(MNode* pNode)
{
	if (MDirectionalLight * pDirLight = dynamic_cast<MDirectionalLight*>(pNode))
		m_vDirectionalLight.push_back(pDirLight);

	else if (MPointLight * pPotLight = dynamic_cast<MPointLight*>(pNode))
		m_vPointLight.push_back(pPotLight);

	else if (MMeshInstance * pMeshIns = dynamic_cast<MMeshInstance*>(pNode))
		RecordMeshInstance(pMeshIns);
}

void MIScene::OnNodeExit(MNode* pNode)
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

	else if (MMeshInstance * pMeshIns = dynamic_cast<MMeshInstance*>(pNode))
		CancelRecordMeshInstance(pMeshIns);
}

void MIScene::RecordMeshInstance(MMeshInstance* pMeshInstance)
{
	if (!pMeshInstance->GetMaterial())
		return;

	std::vector<MaterialMeshInsGroup*>::iterator iter = std::lower_bound(m_vMatMeshInsGroup.begin(), m_vMatMeshInsGroup.end(), pMeshInstance->GetMaterial(), [](MaterialMeshInsGroup* a, MMaterial* b) {return a->pMat < b; });
	MaterialMeshInsGroup* pGroup = nullptr;
	if (iter == m_vMatMeshInsGroup.end())
	{
		pGroup = new MaterialMeshInsGroup();
		pGroup->pMat = pMeshInstance->GetMaterial();
		m_vMatMeshInsGroup.push_back(pGroup);
	}
	else
	{
		pGroup = *iter;
	}

	for (MMeshInstance* pMeshIns : pGroup->vMeshIns)
	{
		if (pMeshIns == pMeshInstance)
			return;
	}
	pGroup->vMeshIns.push_back(pMeshInstance);
}

void MIScene::CancelRecordMeshInstance(MMeshInstance* pMeshInstance)
{
	if (!pMeshInstance->GetMaterial())
		return;

	MMaterial* pMaterial = pMeshInstance->GetMaterial();
	std::vector<MaterialMeshInsGroup*>::iterator iter = std::lower_bound(m_vMatMeshInsGroup.begin(), m_vMatMeshInsGroup.end(), pMaterial, [](MaterialMeshInsGroup* a, MMaterial* b) {return a->pMat < b; });
	if (iter == m_vMatMeshInsGroup.end())
		return;
	
	MaterialMeshInsGroup* pGroup = *iter;

	std::vector<MMeshInstance*>::iterator it = find(pGroup->vMeshIns.begin(), pGroup->vMeshIns.end(), pMeshInstance);
	if (it != pGroup->vMeshIns.end())
		pGroup->vMeshIns.erase(it);
}

void MIScene::DrawMeshInstance(MIRenderer* pRenderer, MIViewport* pViewport)
{
	for (MaterialMeshInsGroup* pGroup : m_vMatMeshInsGroup)
	{
		MMaterial* pMaterial = pGroup->pMat;
		//使用材质
		pRenderer->SetUseMaterial(pMaterial);
		//更新纹理资源，纹理资源只更新一次，节省性能
		pRenderer->UpdateMaterialResource();

		for (MMeshInstance* pMeshIns : pGroup->vMeshIns)
		{
			if(!pMeshIns->GetVisibleRecursively())
				continue;

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

			pRenderer->UpdateMaterialParam();
			pRenderer->DrawMesh(pMeshIns->GetMesh());
		}
	}
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
			pRenderer->UpdateMaterialParam();
			pRenderer->UpdateMaterialResource();
			pRenderer->DrawMesh(pMeshIns->GetMesh());
		}
		
	}
}

void MIScene::Render(MIRenderer* pRenderer, MIViewport* pViewport)
{
	DrawMeshInstance(pRenderer, pViewport);
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
