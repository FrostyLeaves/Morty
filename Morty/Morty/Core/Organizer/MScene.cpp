#include "MScene.h"
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
#include "MViewport.h"
#include "MTransformCoord.h"
#include "MModelInstance.h"
#include "MBounds.h"
#include "MModelResource.h"
#include "MPainter.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "MMaterialResource.h"
#include "MInputNode.h"
#include "MTexture.h"
#include "MTextureRenderTarget.h"
#include "MModelInstance.h"

#include "MModelInstance.h"
#include "MSkeleton.h"

#include "MShadowTextureRenderTarget.h"

#include <algorithm>

MTypeIdentifierImplement(MScene, MObject)

MScene::MScene()
	: MObject()
	, m_pRootNode(nullptr)
	, m_pSkyBox(nullptr)
	, m_pTransformCoord3D(nullptr)
	, m_pShadowDepthMapRenderTarget(nullptr)
	, m_vViewports()
{
	
}

void MScene::OnCreated()
{
	MObject::OnCreated();

	m_pSkyBox = m_pEngine->GetObjectManager()->CreateObject<MSkyBox>();

	m_pTransformCoord3D = m_pEngine->GetObjectManager()->CreateObject<MTransformCoord3D>();

	InitShadowMapRenderTarget();
}

void MScene::InitShadowMapRenderTarget()
{
	if (m_pShadowDepthMapRenderTarget)
		return;

	MMaterialResource* pShadowMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW);
	MMaterial* pStaticMaterial = pShadowMaterialRes->GetMaterialTemplate();

	MMaterialResource* pShadowWithAnimMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_SHADOW_ANIM);
	MMaterial* pAnimMaterial = pShadowWithAnimMaterialRes->GetMaterialTemplate();

	m_pShadowDepthMapRenderTarget = m_pEngine->GetObjectManager()->CreateObject<MShadowTextureRenderTarget>();

	m_pShadowDepthMapRenderTarget->SetScene(this);
}

MBoundsAABB* MScene::GetDirectionalShadowSceneAABB()
{
	Vector3 v3Min(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (MaterialMeshInsGroup* pGroup : m_vMatMeshInsGroup)
	{
		for (MIMeshInstance* pMeshIns : pGroup->vMeshIns)
		{
			if (pMeshIns->GetShadowType() != MIMeshInstance::ENone && pMeshIns->GetVisibleRecursively())
			{
				const MBoundsAABB* pBounds = pMeshIns->GetBoundsAABB();
				pBounds->UnionMinMax(v3Min, v3Max);
			}
		}
	}

	MBoundsAABB* pResult = new MBoundsAABB();
	pResult->SetMinMax(v3Min, v3Max);

	return pResult;
}

void MScene::AddAttachedViewport(MViewport* pViewport)
{
	for (MViewport* pv : m_vViewports)
		if (pv == pViewport)
			return;

	m_vViewports.push_back(pViewport);


}

void MScene::RemoveAttachedViewport(MViewport* pViewport)
{
	std::vector<MViewport*>::iterator iter = std::find(m_vViewports.begin(), m_vViewports.end(), pViewport);
	if (iter != m_vViewports.end())
	{
		m_vViewports.erase(iter);
	}
}

MDirectionalLight* MScene::FindActiveDirectionLight()
{
	if (m_vDirectionalLight.empty())
		return nullptr;

	return m_vDirectionalLight.back();
}

void MScene::FindActivePointLights(const Vector3& v3WorldPosition, std::vector<MPointLight*>& vPointLights)
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

void MScene::OnNodeEnter(MNode* pNode)
{
	if (MModelInstance* pModelIns = pNode->DynamicCast<MModelInstance>())
	{
		if (pModelIns->GetSkeleton())
			m_vAnimationalModelInstances.push_back(pModelIns);
		else
			m_vStaticModelInstances.push_back(pModelIns);

	}

	else if (MDirectionalLight* pDirLight = pNode->DynamicCast<MDirectionalLight>())
	{
		m_vDirectionalLight.push_back(pDirLight);
	}

	else if (MPointLight* pPotLight = pNode->DynamicCast<MPointLight>())
	{
		m_vPointLight.push_back(pPotLight);
	}
	else if (MIMeshInstance* pMeshIns = pNode->DynamicCast<MIMeshInstance>())
		RecordMeshInstance(pMeshIns);

	else if (MCamera* pCamera = pNode->DynamicCast<MCamera>())
	{
		for (MViewport* pViewport : m_vViewports)
		{
			if (pViewport->IsUseDefaultCamera())
				pViewport->SetCamera(pCamera);
		}
	}

	else if (MInputNode* pInputNode = pNode->DynamicCast<MInputNode>())
		RecordInputNode(pInputNode);
}

void MScene::OnNodeExit(MNode* pNode)
{
	if (MModelInstance* pModelIns = pNode->DynamicCast<MModelInstance>())
	{
		if (pModelIns->GetSkeleton())
		{
			std::vector<MModelInstance*>::iterator iter = std::find(m_vAnimationalModelInstances.begin(), m_vAnimationalModelInstances.end(), pModelIns);
			if (m_vAnimationalModelInstances.end() != iter)
				m_vAnimationalModelInstances.erase(iter);
		}
		else
		{
			std::vector<MModelInstance*>::iterator iter = std::find(m_vStaticModelInstances.begin(), m_vStaticModelInstances.end(), pModelIns);
			if (m_vStaticModelInstances.end() != iter)
				m_vStaticModelInstances.erase(iter);
		}
	}

	else if (MDirectionalLight* pDirLight = pNode->DynamicCast<MDirectionalLight>())
	{
		std::vector<MDirectionalLight*>::iterator iter = std::find(m_vDirectionalLight.begin(), m_vDirectionalLight.end(), pDirLight);
		if(m_vDirectionalLight.end() != iter)
			m_vDirectionalLight.erase(iter);
	}

	else if (MPointLight* pPotLight = pNode->DynamicCast<MPointLight>())
	{
		std::vector<MPointLight*>::iterator iter = std::find(m_vPointLight.begin(), m_vPointLight.end(), pPotLight);
		if (m_vPointLight.end() != iter)
			m_vPointLight.erase(iter);
	}

	else if (MIMeshInstance* pMeshIns = pNode->DynamicCast<MIMeshInstance>())
		CancelRecordMeshInstance(pMeshIns);

	else if (MCamera* pCamera = pNode->DynamicCast<MCamera>())
	{
		for (MViewport* pViewport : m_vViewports)
		{
			if (pViewport->GetCamera() == pCamera)
				pViewport->SetCamera(nullptr);
		}
	}

	else if (MInputNode* pInputNode = pNode->DynamicCast<MInputNode>())
		CancelRecordInputNode(pInputNode);
}

void MScene::RecordMeshInstance(MIMeshInstance* pMeshInstance)
{
	if (!pMeshInstance->GetMaterial())
		return;

	MMaterial* pMaterial = pMeshInstance->GetMaterial();
	std::vector<MaterialMeshInsGroup*>::iterator iter = std::lower_bound(m_vMatMeshInsGroup.begin(), m_vMatMeshInsGroup.end(), pMaterial, [](MaterialMeshInsGroup* a, MMaterial* b) {return a->pMat < b; });
	MaterialMeshInsGroup* pGroup = nullptr;
	if (iter == m_vMatMeshInsGroup.end())
	{
		pGroup = new MaterialMeshInsGroup();
		pGroup->pMat = pMeshInstance->GetMaterial();
		m_vMatMeshInsGroup.push_back(pGroup);
	}
	else if ((*iter)->pMat != pMaterial)
	{
			pGroup = new MaterialMeshInsGroup();
			pGroup->pMat = pMeshInstance->GetMaterial();
			m_vMatMeshInsGroup.insert(iter, pGroup);
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

void MScene::CancelRecordMeshInstance(MIMeshInstance* pMeshInstance)
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

	if (pGroup->vMeshIns.empty())
		m_vMatMeshInsGroup.erase(iter);
}

void MScene::RecordInputNode(MInputNode* pInputNode)
{
	std::vector<MInputNode*>::iterator iter = std::lower_bound(m_vInputNodes.begin(), m_vInputNodes.end(), pInputNode, [](MInputNode* a, MInputNode* b) {return a->GetObjectID() < b->GetObjectID(); });
	if (iter == m_vInputNodes.end())
	{
		m_vInputNodes.push_back(pInputNode);
	}
	else
	{
		m_vInputNodes.insert(iter, pInputNode);
	}
}

void MScene::CancelRecordInputNode(MInputNode* pInputNode)
{
	std::vector<MInputNode*>::iterator iter = std::lower_bound(m_vInputNodes.begin(), m_vInputNodes.end(), pInputNode, [](MInputNode* a, MInputNode* b) {return a->GetObjectID() < b->GetObjectID(); });
	if (iter == m_vInputNodes.end())
		return;

	m_vInputNodes.erase(iter);
}

void MScene::GenerateShadowMap(MIRenderer* pRenderer, MViewport* pViewport)
{
	if (nullptr == m_pShadowDepthMapRenderTarget)
		return;

	m_pShadowDepthMapRenderTarget->SetSourceViewport(pViewport);

	pRenderer->Render(m_pShadowDepthMapRenderTarget);

}

void MScene::DrawMeshInstance(MIRenderer* pRenderer, MViewport* pViewport)
{
	MDirectionalLight* pDirectionalLight = FindActiveDirectionLight();
	Matrix4 matLightInvProj = pViewport->GetLightInverseProjection(pDirectionalLight);

	for (MaterialMeshInsGroup* pGroup : m_vMatMeshInsGroup)
	{
		MMaterial* pMaterial = pGroup->pMat;
		//使用材质
		if(!pRenderer->SetUseMaterial(pMaterial))
			continue;
		
		//纹理资源
		std::vector<MShaderTextureParam>& vPixTexParams = pMaterial->GetPixelTextureParams();
		for (MShaderTextureParam& param : vPixTexParams)
		{
			if (param.unCode == SHADER_PARAM_CODE_SHADOW_MAP)
			{
				param.pTexture = m_pShadowDepthMapRenderTarget->m_pDepthTexture;
				break;
			}
		}
		//更新纹理资源，纹理资源只更新一次，节省性能
		pRenderer->UpdateMaterialResource();


		//VS常量
		std::vector<MShaderParam>& vVtxParams = pMaterial->GetVertexShaderParams();
		for (MShaderParam& param : vVtxParams)
		{
			if (param.unCode == SHADER_PARAM_CODE_WORLD_MATRIX)
			{
				MStruct& cStruct = *param.var.GetStruct();
				cStruct[0] = pViewport->GetCameraInverseProjection();
				cStruct[1] = matLightInvProj;

				param.SetDirty();
				continue;
			}

			else if (param.unCode == SHADER_PARAM_CODE_WORLDINFO)
			{
				if (pDirectionalLight)
				{
					(*param.var.GetStruct())[0] = pDirectionalLight->GetWorldDirection();
				}

				(*param.var.GetStruct())[1] = pViewport->GetCamera()->GetWorldPosition();

				param.SetDirty();
				continue;
			}
		}
		
		//PS常量
		std::vector<MShaderParam>& vPixParams = pMaterial->GetPixelShaderParams();
		for (MShaderParam& param : vPixParams)
		{
			if (param.unCode == SHADER_PARAM_CODE_LIGHT)
			{
				if (pDirectionalLight)
				{
					MVariant& cDirectionLight = (*param.var.GetStruct())[0];
					{
						MStruct& cLightStruct = *cDirectionLight.GetStruct();
						{
							cLightStruct[0] = pDirectionalLight->GetAmbientColor().ToVector3();
							cLightStruct[1] = pDirectionalLight->GetDiffuseColor().ToVector3();
							cLightStruct[2] = pDirectionalLight->GetSpecularColor().ToVector3();
						}
					}
				}

				param.SetDirty();

				continue;
			}
			else if (param.unCode == SHADER_PARAM_CODE_WORLDINFO)
			{
				if (pDirectionalLight)
				{
					(*param.var.GetStruct())[0] = pDirectionalLight->GetWorldDirection();
				}

				(*param.var.GetStruct())[1] = pViewport->GetCamera()->GetWorldPosition();

				param.SetDirty();
				continue;
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
				if (param.unCode == SHADER_PARAM_CODE_MESH_MATRIX)
				{
					Matrix4 worldTrans = pMeshIns->GetWorldTransform();

					//Transposed and Inverse.
					Matrix3 matNormal(worldTrans.Transposed().Inverse(), 3, 3);

					MStruct& cStruct = *param.var.GetStruct();
					cStruct[0] = worldTrans;
					cStruct[1] = matNormal;

					param.SetDirty();
					continue;
				}
				else if (param.unCode == SHADER_PARAM_CODE_ANIMATION)
				{
					if (MModelInstance* pModel = dynamic_cast<MModelInstance*>(pMeshIns->GetParent()))
					{
						if (nullptr == pModel->GetSkeleton())
							continue;

						MStruct& cAnimationStruct = *param.var.GetStruct();
						MVariantArray& cBonesArray = *cAnimationStruct[0].GetArray() ;

						const std::vector<MBone*>& bones = pModel->GetSkeleton()->GetAllBones();
						unsigned int size = bones.size();
						if (size > MBONES_MAX_NUMBER)
							size = MBONES_MAX_NUMBER;
						for (unsigned int i = 0; i < size; ++i)
						{
							cBonesArray[i] = bones[i]->GetTransformInModelWorld();
						}

						param.SetDirty();
					}
				}
			}

			for (MShaderParam& param : vPixParams)
			{
				if (param.unCode == SHADER_PARAM_CODE_LIGHT)
				{
					MVariant& varPointLights = (*param.var.GetStruct())[1];
					{
						MVariantArray& vPointLights = *varPointLights.GetArray() ;
						for (unsigned int i = 0; i < vPointLights.GetMemberCount(); ++i)
						{
							if (MPointLight* pLight = vActivePointLights[i])
							{
								MStruct& cPointLight = *vPointLights[i].GetStruct();
								cPointLight[0] = pLight->GetWorldPosition();
								cPointLight[1] = pLight->GetAmbientColor().ToVector3();
								cPointLight[2] = pLight->GetDiffuseColor().ToVector3();
								cPointLight[3] = pLight->GetSpecularColor().ToVector3();

								cPointLight[4] = 1.0f;
								cPointLight[5] = 0.022f;
								cPointLight[6] = 0.0019f;
								
							}
						}
					}

					param.SetDirty();
					continue;
				}
			}

			pRenderer->UpdateMaterialParam();
			pRenderer->DrawMesh(pMeshIns->GetMesh());
		}
	}
}

void MScene::DrawModelInstance(MIRenderer* pRenderer, MViewport* pViewport)
{
	for (MModelInstance* pModelIns : m_vStaticModelInstances)
	{
		if (pModelIns->GetDrawBoundingBox())
		{
			DrawBoundingBox(pRenderer, pViewport, pModelIns);
		}
	}

	for (MModelInstance* pModelIns : m_vAnimationalModelInstances)
	{
		if (pModelIns->GetDrawBoundingBox())
		{
			DrawBoundingBox(pRenderer, pViewport, pModelIns);
		}
	}
}

void MScene::DrawSkyBox(MIRenderer* pRenderer, MViewport* pViewport)
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
				if (param.unCode == SHADER_PARAM_CODE_MESH_MATRIX)
				{
					MStruct& cStruct = *param.var.GetStruct();
					Matrix4 mat(Matrix4::IdentityMatrix);
					Vector3 camPos = pViewport->GetCamera()->GetPosition();
					mat.m[0][3] = camPos.x;
					mat.m[1][3] = camPos.y;
					mat.m[2][3] = camPos.z;
					cStruct[0] = mat;
					
					param.SetDirty();
					continue;
				}
				else if (param.unCode == SHADER_PARAM_CODE_WORLD_MATRIX)
				{
					MStruct& cStruct = *param.var.GetStruct();
					cStruct[0] = pViewport->GetCameraInverseProjection();

					param.SetDirty();
					continue;
				}
			}
			if (pRenderer->SetUseMaterial(pMaterial))
			{
				pRenderer->UpdateMaterialParam();
				pRenderer->UpdateMaterialResource();
				pRenderer->DrawMesh(pMeshIns->GetMesh());
			}
		}
		
	}
}

void MScene::DrawPainter(MIRenderer* pRenderer, MViewport* pViewport)
{
	m_pTransformCoord3D->Render(pRenderer, pViewport);
}

void MScene::DrawBoundingBox(MIRenderer* pRenderer, MViewport* pViewport, MModelInstance* pModelIns)
{
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes->GetMaterialTemplate();
	if (!pRenderer->SetUseMaterial(pMaterial))
		return;

	std::vector<MShaderParam>& vVtxParams = pMaterial->GetVertexShaderParams();
	for (MShaderParam& param : vVtxParams)
	{
		if (param.unCode == SHADER_PARAM_CODE_WORLD_MATRIX)
		{
			MStruct& cStruct = *param.var.GetStruct();
			cStruct[0] = pViewport->GetCameraInverseProjection();
			
			param.SetDirty();
			continue;
		}
	}

	pRenderer->UpdateMaterialParam();

	const MBoundsAABB* pAABB = pModelIns->GetBoundsAABB();

	const Vector3& obmin = pAABB->m_v3MinPoint;
	const Vector3& obmax = pAABB->m_v3MaxPoint;

	Vector3 list[] = {
		Vector3(obmin.x, obmin.y, obmin.z),
		Vector3(obmax.x, obmin.y, obmin.z),
		Vector3(obmax.x, obmax.y, obmin.z),
		Vector3(obmin.x, obmax.y, obmin.z),

		Vector3(obmin.x, obmin.y, obmax.z),
		Vector3(obmax.x, obmin.y, obmax.z),
		Vector3(obmax.x, obmax.y, obmax.z),
		Vector3(obmin.x, obmax.y, obmax.z),
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
}

void MScene::DrawCameraFrustum(MIRenderer* pRenderer, MViewport* pViewport, MCamera* pCamera)
{
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes->GetMaterialTemplate();
	if (!pRenderer->SetUseMaterial(pMaterial))
		return;

	std::vector<MShaderParam>& vVtxParams = pMaterial->GetVertexShaderParams();
	for (MShaderParam& param : vVtxParams)
	{
		if (param.unCode == SHADER_PARAM_CODE_WORLD_MATRIX)
		{
			MStruct& cStruct = *param.var.GetStruct();
			cStruct[0] = pViewport->GetCameraInverseProjection();
			param.SetDirty();
			continue;
		}
	}

	pRenderer->UpdateMaterialParam();

	Vector3 list[8];
	pViewport->GetCameraFrustum(list[0], list[1], list[2], list[3], list[4], list[5], list[6], list[7]);


	Vector2 begin, end;
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 0; i < 2; ++i)
		{
			MPainter2DLine3D line(list[j + i * 4], list[(j + 1) % 4 + i * 4], MColor(i == 0 ? 0 : 1, 1, 1, 1), 1.0f);

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

}

void MScene::Render(MIRenderer* pRenderer, MViewport* pViewport)
{
	GenerateShadowMap(pRenderer, pViewport);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, pViewport->GetWidth(), pViewport->GetHeight(), 0.0f, 1.0f);
	DrawPainter(pRenderer, pViewport);
	DrawMeshInstance(pRenderer, pViewport);
	DrawModelInstance(pRenderer, pViewport);
}

void MScene::Input(MInputEvent* pEvent, MViewport* pViewport)
{
	m_pTransformCoord3D->Input(pEvent, pViewport);

	for (MInputNode* pNode : m_vInputNodes)
	{
		if (pNode->Input(pEvent, pViewport))
			break;
	}
}

MScene::~MScene()
{
	if (m_pShadowDepthMapRenderTarget)
	{
		m_pEngine->GetObjectManager()->RemoveObject(m_pShadowDepthMapRenderTarget->GetObjectID());
		m_pShadowDepthMapRenderTarget = nullptr;
	}
}

void MScene::SetRootNode(MNode* pRootNode)
{
	m_pRootNode = pRootNode;
	if (pRootNode)
	{
		pRootNode->SetAttachedScene(this);
	}
}
