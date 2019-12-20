#include "MIScene.h"
#include "MEngine.h"
#include "MCamera.h"
#include "MSkyBox.h"

#include "MDirectionalLight.h"
#include "MPointLight.h"

#include "MStaticMeshInstance.h"
#include "MVertex.h"
#include "MMaterial.h"
#include "MIRenderer.h"
#include "MIRenderView.h"
#include "MIViewport.h"
#include "MTransformCoord.h"
#include "MModelInstance.h"
#include "MBounds.h"
#include "MModelResource.h"
#include "MPainter.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "MMaterialResource.h"

#include "MModelInstance.h"
#include "MSkeleton.h"

#include <algorithm>

MIScene::MIScene()
	: MObject()
	, m_pRootNode(nullptr)
	, m_pSkyBox(nullptr)
	, m_pTransformCoord3D(nullptr)
	, m_vViewports()
{
	
}

#include "MInputManager.h"
void MIScene::OnCreated()
{
	MObject::OnCreated();

	m_pSkyBox = m_pEngine->GetObjectManager()->CreateObject<MSkyBox>();

	m_pTransformCoord3D = m_pEngine->GetObjectManager()->CreateObject<MTransformCoord3D>();

}

void MIScene::AddAttachedViewport(MIViewport* pViewport)
{
	for (MIViewport* pv : m_vViewports)
		if (pv == pViewport)
			return;

	m_vViewports.push_back(pViewport);


}

void MIScene::RemoveAttachedViewport(MIViewport* pViewport)
{
	std::vector<MIViewport*>::iterator iter = std::find(m_vViewports.begin(), m_vViewports.end(), pViewport);
	if (iter != m_vViewports.end())
	{
		m_vViewports.erase(iter);
	}
}

MDirectionalLight* MIScene::FindActiveDirectionLight()
{
	if (m_vDirectionalLight.empty())
		return nullptr;

	return m_vDirectionalLight.back();
}

void MIScene::FindActivePointLights(const Vector3& v3WorldPosition, std::vector<MPointLight*>& vPointLights)
{
	auto compareFunc = [v3WorldPosition](MPointLight* a, MPointLight* b) {
		return (a->GetWorldPosition() - v3WorldPosition).Length() < (b->GetWorldPosition() - v3WorldPosition).Length();
	};

	if (m_vPointLight.size() <= vPointLights.size())
	{
		std::copy(m_vPointLight.begin(), m_vPointLight.end(), vPointLights.begin());
	}
	else if (vPointLights.size() * 2 < m_vPointLight.size())
	{
		std::fill(vPointLights.begin(), vPointLights.end(), nullptr);
		for (MPointLight* pLight : m_vPointLight)
		{
			for (std::vector<MPointLight*>::iterator iter = vPointLights.begin(); iter != vPointLights.end(); ++iter)
			{
				if (*iter == nullptr)
				{
					(*iter) = pLight;
					break;
				}
				else if (compareFunc(pLight, *iter))
				{
					for (std::vector<MPointLight*>::iterator nextIter = vPointLights.end() - 1; nextIter != iter; --nextIter)
						(*nextIter) = *(nextIter - 1);
					(*iter) = pLight;
					break;
				}
			}
		}
	}
	else
	{
		std::sort(m_vPointLight.begin(), m_vPointLight.end(), compareFunc);
		std::copy(m_vPointLight.begin(), m_vPointLight.begin() + vPointLights.size(), vPointLights.begin());
	}
	
}

void MIScene::OnNodeEnter(MNode* pNode)
{
	if (MDirectionalLight * pDirLight = dynamic_cast<MDirectionalLight*>(pNode))
		m_vDirectionalLight.push_back(pDirLight);

	else if (MPointLight * pPotLight = dynamic_cast<MPointLight*>(pNode))
		m_vPointLight.push_back(pPotLight);

	else if (MStaticMeshInstance * pMeshIns = dynamic_cast<MStaticMeshInstance*>(pNode))
		RecordMeshInstance(pMeshIns);

	else if (MCamera* pCamera = dynamic_cast<MCamera*>(pNode))
	{
		for (MIViewport* pViewport : m_vViewports)
		{
			if (pViewport->IsUseDefaultCamera())
				pViewport->SetCamera(pCamera);
		}
	}
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

	else if (MStaticMeshInstance * pMeshIns = dynamic_cast<MStaticMeshInstance*>(pNode))
		CancelRecordMeshInstance(pMeshIns);

	else if (MCamera* pCamera = dynamic_cast<MCamera*>(pNode))
	{
		for (MIViewport* pViewport : m_vViewports)
		{
			if (pViewport->GetCamera() == pCamera)
				pViewport->SetCamera(nullptr);
		}
	}

}

void MIScene::RecordMeshInstance(MIMeshInstance* pMeshInstance)
{
	if (!pMeshInstance->GetMaterial())
		return;

	std::vector<MaterialMeshInsGroup*>::iterator iter = std::lower_bound(m_vMatMeshInsGroup.begin(), m_vMatMeshInsGroup.end(), pMeshInstance, [](MaterialMeshInsGroup* a, MIMeshInstance* b) {return a->pMat < b->GetMaterial(); });
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

	std::vector<MIMeshInstance*>::iterator it = find(pGroup->vMeshIns.begin(), pGroup->vMeshIns.end(), pMeshInstance);
	if (it != pGroup->vMeshIns.end())
		return;

	pGroup->vMeshIns.push_back(pMeshInstance);
}

void MIScene::CancelRecordMeshInstance(MIMeshInstance* pMeshInstance)
{
	if (!pMeshInstance->GetMaterial())
		return;

	MMaterial* pMaterial = pMeshInstance->GetMaterial();
	std::vector<MaterialMeshInsGroup*>::iterator iter = std::lower_bound(m_vMatMeshInsGroup.begin(), m_vMatMeshInsGroup.end(), pMaterial, [](MaterialMeshInsGroup* a, MMaterial* b) {return a->pMat < b; });
	if (iter == m_vMatMeshInsGroup.end())
		return;
	
	MaterialMeshInsGroup* pGroup = *iter;

	std::vector<MIMeshInstance*>::iterator it = find(pGroup->vMeshIns.begin(), pGroup->vMeshIns.end(), pMeshInstance);
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


		MDirectionalLight* pDirectionalLight = FindActiveDirectionLight();

		std::vector<MShaderParam>& vVtxParams = pMaterial->GetVertexShaderParams();
		std::vector<MShaderParam>& vPixParams = pMaterial->GetPixelShaderParams();
		for (MShaderParam& param : vPixParams)
		{
			if (param.strName == "cbLights")
			{
				if (pDirectionalLight)
				{
					if (MVariant* pDirectionLight = param.var.GetByType<MStruct>()->FindMember("U_dirLight"))
					{
						if (MStruct* pLightStruct = pDirectionLight->GetByType<MStruct>())
						{
							pLightStruct->SetMember("f3Direction", pDirectionalLight->GetDirection());
							pLightStruct->SetMember("f3Ambient", pDirectionalLight->GetAmbientColor().ToVector3());
							pLightStruct->SetMember("f3Diffuse", pDirectionalLight->GetDiffuseColor().ToVector3());
							pLightStruct->SetMember("f3Specular", pDirectionalLight->GetSpecularColor().ToVector3());
						}
					}
				}
			}
			else if (param.strName == "cbWorldInfo")
			{
				if (MStruct* pStruct = param.var.GetByType<MStruct>())
				{
					pStruct->SetMember("U_f3CameraWorldPos", pViewport->GetCamera()->GetWorldPosition());
				}
			}
		}

		for (MIMeshInstance* pMeshIns : pGroup->vMeshIns)
		{
			if(!pMeshIns->GetVisibleRecursively())
				continue;

			std::vector<MPointLight*> vActivePointLights(4);
			FindActivePointLights(pMeshIns->GetWorldPosition(), vActivePointLights);

			for (MShaderParam& param : vVtxParams)
			{
				if (param.strName == "cbSpace")
				{
					Matrix4 worldTrans = pMeshIns->GetWorldTransform();

					//Transposed and Inverse. but hlsl has been transporsed.
					Matrix3 matNormal(worldTrans.Transposed().Inverse(), 3, 3);

					MStruct* pSpaceStruct = param.var.GetByType<MStruct>();
					pSpaceStruct->SetMember("U_matWorld", worldTrans);
					pSpaceStruct->SetMember("U_matCamProj", pViewport->GetCameraInverseProjection());

					pSpaceStruct->SetMember("U_matNormal", matNormal);
				}
				else if (param.strName == "cbAnimation")
				{
					if (MModelInstance* pModel = dynamic_cast<MModelInstance*>(pMeshIns->GetParent()))
					{
						MStruct* pAnimationStruct = param.var.GetByType<MStruct>();
						MVariant* pVariant = pAnimationStruct->FindMember("U_vBonesMatrix");
						MVariantArray* pBonesArray = pVariant->GetByType<MVariantArray>();

						const std::vector<MBone*>& bones = pModel->GetSkeleton()->GetAllBones();
						unsigned int size = bones.size();
						if (size > MBONES_MAX_NUMBER)
							size = MBONES_MAX_NUMBER;
						if (pModel->GetSkeletalAnimationController())
						{
							for (unsigned int i = 0; i < size; ++i)
							{
								(*pBonesArray)[i] = bones[i]->GetTransformInModelWorld();
							}
						}
						else
						{
							for (unsigned int i = 0; i < size; ++i)
							{
								(*pBonesArray)[i] = Matrix4::IdentityMatrix;
							}
						}
					}
				}
			}

			
			for (MShaderParam& param : vPixParams)
			{
				if (param.strName == "cbLights")
				{
					MVariant* varPointLights = param.var.GetByType<MStruct>()->FindMember("U_pointLights");
					if (varPointLights)
					{
						MVariantArray& vPointLights = *varPointLights->GetByType<MVariantArray>();
						for (unsigned int i = 0; i < vPointLights.GetMemberCount(); ++i)
						{
							if (MStruct* pPointLight = vPointLights[i].GetByType<MStruct>())
							{
								if (MPointLight* pLight = vActivePointLights[i])
								{
									pPointLight->SetMember("f3WorldPosition", pLight->GetWorldPosition());
									pPointLight->SetMember("f3Ambient", pLight->GetAmbientColor().ToVector3());
									pPointLight->SetMember("f3Diffuse", pLight->GetDiffuseColor().ToVector3());
									pPointLight->SetMember("f3Specular", pLight->GetSpecularColor().ToVector3());

									pPointLight->SetMember("fConstant", 1.0f);
									pPointLight->SetMember("fLinear", 0.022f);
									pPointLight->SetMember("fQuadratic", 0.0019f);
								}
							}
						}
					}
				}
				else if (param.strName == "cbWorldInfo")
				{
					if (MStruct* pStruct = param.var.GetByType<MStruct>())
					{
						pStruct->SetMember("U_f3CameraWorldPos", pViewport->GetCamera()->GetWorldPosition());
					}
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
		if (MStaticMeshInstance* pMeshIns = m_pSkyBox->GetMeshInstance())
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

void MIScene::DrawPainter(MIRenderer* pRenderer, MIViewport* pViewport)
{
	m_pTransformCoord3D->Render(pRenderer, pViewport);
}

void MIScene::DrawBoundingBox(MIRenderer* pRenderer, MIViewport* pViewport, MModelInstance* pSpatial)
{
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes->GetMaterialTemplate();

	std::vector<MShaderParam>& vVtxParams = pMaterial->GetVertexShaderParams();
	for (MShaderParam& param : vVtxParams)
	{
		if (param.strName == "cbSpace")
		{
			MStruct* pSpaceStruct = param.var.GetByType<MStruct>();
			pSpaceStruct->SetMember("MatCamProj", pViewport->GetCameraInverseProjection());
			break;
		}
	}

	pRenderer->SetUseMaterial(pMaterial);
	pRenderer->UpdateMaterialParam();

	MModelResource* pModelResource = dynamic_cast<MModelResource*>(pSpatial->GetResource());
	const MBoundsOBB* pObb = pModelResource->GetOBB();

	Matrix4 mat4World = pSpatial->GetWorldTransform();
	const Vector3& obmin = pObb->m_v3MinPoint;
	const Vector3& obmax = pObb->m_v3MaxPoint;

	Vector3 list[] = {
		mat4World * pObb->ConvertFromOBB(Vector3(obmin.x, obmin.y, obmin.z)),
		mat4World* pObb->ConvertFromOBB(Vector3(obmax.x, obmin.y, obmin.z)),
		mat4World* pObb->ConvertFromOBB(Vector3(obmax.x, obmax.y, obmin.z)),
		mat4World* pObb->ConvertFromOBB(Vector3(obmin.x, obmax.y, obmin.z)),

		mat4World* pObb->ConvertFromOBB(Vector3(obmin.x, obmin.y, obmax.z)),
		mat4World* pObb->ConvertFromOBB(Vector3(obmax.x, obmin.y, obmax.z)),
		mat4World* pObb->ConvertFromOBB(Vector3(obmax.x, obmax.y, obmax.z)),
		mat4World* pObb->ConvertFromOBB(Vector3(obmin.x, obmax.y, obmax.z)),
	};

	Vector2 begin, end;
	for (int j = 0; j < 4; ++j)
	{
		for(int i = 0; i < 2; ++ i)
		{
			MPainter2DLine3D line(list[j + i * 4], list[(j + 1) % 4 + i * 4], MColor(1, 1, 1, 1), 1.0f);

			MMesh<MPainterVertex> meshs;
			if (line.FillData(pViewport, meshs))
			{
				pRenderer->DrawMesh(&meshs);
				meshs.DestroyBuffer(m_pEngine->GetDevice());
			}
		}

		MPainter2DLine3D line(list[j], list[(j + 4)], MColor(1, 1, 1, 1), 1.0f);
		MMesh<MPainterVertex> meshs;
		if (line.FillData(pViewport, meshs))
		{
			pRenderer->DrawMesh(&meshs);
			meshs.DestroyBuffer(m_pEngine->GetDevice());
		}
	}

	pRenderer->UpdateMaterialParam();
}

void MIScene::Render(MIRenderer* pRenderer, MIViewport* pViewport)
{
	DrawPainter(pRenderer, pViewport);
	MModelInstance* pSpat = dynamic_cast<MModelInstance*>(m_pRootNode->FindFirstChildByName("Teaport"));
	DrawBoundingBox(pRenderer, pViewport, pSpat);
	DrawMeshInstance(pRenderer, pViewport);
//	DrawSkyBox(pRenderer, pViewport);
}

void MIScene::Input(MInputEvent* pEvent, MIViewport* pViewport)
{
	m_pTransformCoord3D->Input(pEvent, pViewport);
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
