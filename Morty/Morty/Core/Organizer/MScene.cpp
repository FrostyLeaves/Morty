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

#if MORTY_RENDER_DATA_STATISTICS
#include "MRenderStatistics.h"
#endif

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
		m_vModelInstances.push_back(pModelIns);
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
		std::vector<MModelInstance*>::iterator iter = std::find(m_vModelInstances.begin(), m_vModelInstances.end(), pModelIns);
		if (m_vModelInstances.end() != iter)
			m_vModelInstances.erase(iter);
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

	//如果当前有Shader使用了ShadowMap，那么进行ShadowMap的更新
	if (SHADER_PARAM_CODE_SHADOW_MAP < MShaderBuffer::s_vTextureParams.size())
	{
		MShaderTextureParam* pShadowParam = MShaderBuffer::s_vTextureParams[SHADER_PARAM_CODE_SHADOW_MAP];
		if (SHADER_PARAM_CODE_SHADOW_MAP == pShadowParam->unCode)
		{
			m_pShadowDepthMapRenderTarget->SetSourceViewport(pViewport);
			pRenderer->Render(m_pShadowDepthMapRenderTarget);

			pShadowParam->pTexture = m_pShadowDepthMapRenderTarget->m_pDepthTexture;
			pRenderer->SetPixelShaderTexture(*pShadowParam);
		}
	}
		
}

void MScene::UpdateShaderSharedParams(MIRenderer* pRenderer, MViewport* pViewport)
{
	MDirectionalLight* pDirectionalLight = FindActiveDirectionLight();
	Matrix4 matLightInvProj = pViewport->GetLightInverseProjection(pDirectionalLight);

	if (MShaderParam* pWorldMatrixParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_WORLD_MATRIX))
	{
		MStruct& cStruct = *pWorldMatrixParam->var.GetStruct();
		cStruct[0] = pViewport->GetCameraInverseProjection();
		cStruct[1] = matLightInvProj;

		pWorldMatrixParam->SetDirty();
		pRenderer->SetVertexShaderParam(*pWorldMatrixParam);
	}

	if (MShaderParam* pWorldInfoParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_WORLDINFO))
	{
		if (pDirectionalLight)
		{
			(*pWorldInfoParam->var.GetStruct())[0] = pDirectionalLight->GetWorldDirection();
		}

		(*pWorldInfoParam->var.GetStruct())[1] = pViewport->GetCamera()->GetWorldPosition();

		pWorldInfoParam->SetDirty();
		pRenderer->SetVertexShaderParam(*pWorldInfoParam);
		pWorldInfoParam->SetDirty();
		pRenderer->SetPixelShaderParam(*pWorldInfoParam);
	}

	if (MShaderParam* pLightParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_LIGHT))
	{
		if (pDirectionalLight)
		{
			MVariant& cDirectionLight = (*pLightParam->var.GetStruct())[0];
			{
				MStruct& cLightStruct = *cDirectionLight.GetStruct();
				{
					cLightStruct[0] = pDirectionalLight->GetDiffuseColor().ToVector3();
					cLightStruct[1] = pDirectionalLight->GetSpecularColor().ToVector3();
				}
			}
		}

		MVariant& varPointLights = (*pLightParam->var.GetStruct())[1];
		MVariant& varValidPointLights = (*pLightParam->var.GetStruct())[2];
		{
			std::vector<MPointLight*> vActivePointLights(MPOINT_LIGHT_MAX_NUMBER);
			FindActivePointLights(pViewport->GetCamera()->GetWorldPosition(), vActivePointLights);
			varValidPointLights = (int)MPOINT_LIGHT_MAX_NUMBER;

			MVariantArray& vPointLights = *varPointLights.GetArray();
			for (unsigned int i = 0; i < vPointLights.GetMemberCount(); ++i)
			{
				if (MPointLight* pLight = vActivePointLights[i])
				{
					MStruct& cPointLight = *vPointLights[i].GetStruct();
					cPointLight[0] = pLight->GetWorldPosition();
					cPointLight[1] = pLight->GetDiffuseColor().ToVector3();
					cPointLight[2] = pLight->GetSpecularColor().ToVector3();

					cPointLight[3] = 1.0f;
					cPointLight[4] = 0.022f;
					cPointLight[5] = 0.0019f;

				}
				else
				{
					varValidPointLights = (int)i;
				}
			}
		}


		pLightParam->SetDirty();
		pRenderer->SetVertexShaderParam(*pLightParam);
		pLightParam->SetDirty();
		pRenderer->SetPixelShaderParam(*pLightParam);
	}
}

void MScene::DrawMeshInstance(MIRenderer* pRenderer, MViewport* pViewport)
{
	MShaderParam* pMeshMatrixParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX);
	if (nullptr == pMeshMatrixParam)
		return;

	MShaderParam* pAnimationParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_ANIMATION);

	for (MaterialMeshInsGroup* pGroup : m_vMatMeshInsGroup)
	{
		MMaterial* pMaterial = pGroup->pMat;
		//使用材质
		if(!pRenderer->SetUseMaterial(pMaterial))
			continue;
		
		//更新材质的纹理资源，最好移到Material的更新
		pRenderer->UpdateMaterialResource();

		for (MIMeshInstance* pMeshIns : pGroup->vMeshIns)
		{
			if(!pMeshIns->GetVisibleRecursively())
				continue;

			Matrix4 worldTrans = pMeshIns->GetWorldTransform();
			//Transposed and Inverse.
			Matrix3 matNormal(worldTrans.Transposed().Inverse(), 3, 3);

			MStruct& cStruct = *pMeshMatrixParam->var.GetStruct();
			cStruct[0] = worldTrans;
			cStruct[1] = matNormal;

			pMeshMatrixParam->SetDirty();
			pRenderer->SetVertexShaderParam(*pMeshMatrixParam);
	
			if (MModelInstance* pModel = dynamic_cast<MModelInstance*>(pMeshIns->GetParent()))
			{
				if (pAnimationParam && pModel->GetSkeleton())
				{

					MStruct& cAnimationStruct = *pAnimationParam->var.GetStruct();
					MVariantArray& cBonesArray = *cAnimationStruct[0].GetArray();

					const std::vector<MBone*>& bones = pModel->GetSkeleton()->GetAllBones();
					unsigned int size = bones.size();
					if (size > MBONES_MAX_NUMBER) size = MBONES_MAX_NUMBER;

					for (unsigned int i = 0; i < size; ++i)
					{
						cBonesArray[i] = bones[i]->GetTransformInModelWorld();
					}

					pAnimationParam->SetDirty();
					pRenderer->SetVertexShaderParam(*pAnimationParam);
				}
			}
			
			pRenderer->UpdateMaterialParam();
			pRenderer->DrawMesh(pMeshIns->GetMesh());
		}
	}
}

void MScene::DrawModelInstance(MIRenderer* pRenderer, MViewport* pViewport)
{
	for (MModelInstance* pModelIns : m_vModelInstances)
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

			if (MShaderParam* pMeshParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX))
			{
				MStruct& cStruct = *pMeshParam->var.GetStruct();
				Matrix4 mat(Matrix4::IdentityMatrix);
				Vector3 camPos = pViewport->GetCamera()->GetPosition();
				mat.m[0][3] = camPos.x;
				mat.m[1][3] = camPos.y;
				mat.m[2][3] = camPos.z;
				cStruct[0] = mat;

				pMeshParam->SetDirty();
				pRenderer->SetVertexShaderParam(*pMeshParam);
			}
			if (MShaderParam* pWorldParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX))
			{
				MStruct& cStruct = *pWorldParam->var.GetStruct();
				cStruct[0] = pViewport->GetCameraInverseProjection();

				pWorldParam->SetDirty();
				pRenderer->SetVertexShaderParam(*pWorldParam);
			}

			if (pRenderer->SetUseMaterial(pMaterial))
			{
				pRenderer->UpdateMaterialResource();
				pRenderer->UpdateMaterialParam();
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

	if (MShaderParam* pWorldParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX))
	{
		MStruct& cStruct = *pWorldParam->var.GetStruct();
		cStruct[0] = pViewport->GetCameraInverseProjection();

		pWorldParam->SetDirty();
		pRenderer->SetVertexShaderParam(*pWorldParam);
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

	if (MShaderParam* pWorldParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX))
	{
		MStruct& cStruct = *pWorldParam->var.GetStruct();
		cStruct[0] = pViewport->GetCameraInverseProjection();

		pWorldParam->SetDirty();
		pRenderer->SetVertexShaderParam(*pWorldParam);
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
#if MORTY_RENDER_DATA_STATISTICS
	MRenderStatistics::GetInstance()->unVertexCount = 0;
#endif

	GenerateShadowMap(pRenderer, pViewport);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, pViewport->GetWidth(), pViewport->GetHeight(), 0.0f, 1.0f);

	UpdateShaderSharedParams(pRenderer, pViewport);
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
