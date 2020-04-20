#include "MRenderSystem.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "MIRenderer.h"

#include "MScene.h"
#include "MViewport.h"
#include "MTexture.h"
#include "MShadowTextureRenderTarget.h"

#include "MPainter.h"
#include "MTransformCoord.h"

#include "MCamera.h"
#include "MSkyBox.h"
#include "Model/MModelInstance.h"
#include "Model/MIModelMeshInstance.h"
#include "Light/MSpotLight.h"
#include "Light/MPointLight.h"
#include "Light/MDirectionalLight.h"

#include "Material/MMaterialResource.h"
#include "Model/MModelResource.h"
#include "Model/MModelMeshStruct.h"


MTypeIdentifierImplement(MRenderSystem, MISystem)

MRenderSystem::MRenderSystem()
	: MISystem()
	, m_pScene(nullptr)
{
}

MRenderSystem::~MRenderSystem()
{
}

void MRenderSystem::Initialize(MScene* pScene)
{
	m_pScene = pScene;
}

void MRenderSystem::Release()
{
	m_pScene = nullptr;
}

void MRenderSystem::Tick(const float& fDelta)
{

}

void MRenderSystem::Render(MIRenderer* pRenderer, MViewport* pViewport)
{
    if (nullptr == m_pScene)
        return;

	MRenderInfo info;

	info.pRenderer = pRenderer;
	info.pViewport = pViewport;

	GenerateShadowMap(info);

	Vector2 v2LeftTop = pViewport->GetLeftTop();
	pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, pViewport->GetWidth(), pViewport->GetHeight(), 0.0f, 1.0f);

	UpdateShaderSharedParams(info);
	DrawPainter(info);
	DrawMeshInstance(info);
	DrawModelInstance(info);
}

void MRenderSystem::GenerateShadowMap(MRenderInfo& info)
{
	MShadowTextureRenderTarget* pShadowRenderTarget = m_pScene->GetShadowRenderTarget();
	if (nullptr == pShadowRenderTarget || nullptr == m_pScene->FindActiveDirectionLight())
	{
		return;
	}
	//如果当前有Shader使用了ShadowMap，那么进行ShadowMap的更新
	if (SHADER_PARAM_CODE_SHADOW_MAP < MShaderBuffer::s_vTextureParams.size())
	{
		MShaderTextureParam* pShadowParam = MShaderBuffer::s_vTextureParams[SHADER_PARAM_CODE_SHADOW_MAP];
		if (SHADER_PARAM_CODE_SHADOW_MAP == pShadowParam->unCode)
		{
			pShadowRenderTarget->SetSourceViewport(info.pViewport);
			info.pRenderer->Render(pShadowRenderTarget);
			pShadowParam->pTexture = pShadowRenderTarget->m_pDepthTexture;
			info.pRenderer->SetPixelShaderTexture(*pShadowParam);
		}
	}

}

void MRenderSystem::UpdateShaderSharedParams(MRenderInfo& info)
{
	MDirectionalLight* pDirectionalLight = m_pScene->FindActiveDirectionLight();
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
		MVariant& varDirLightEnable = (*pLightParam->var.GetStruct())[3];
		if (pDirectionalLight)
		{
			varDirLightEnable = true;
			MVariant& varDirectionLight = (*pLightParam->var.GetStruct())[0];
			{
				MStruct& cLightStruct = *varDirectionLight.GetStruct();
				{
					cLightStruct[0] = pDirectionalLight->GetDiffuseColor().ToVector3();
					cLightStruct[1] = pDirectionalLight->GetSpecularColor().ToVector3();
				}
			}
		}
		else
		{
			varDirLightEnable = false;
		}

		MVariant& varPointLights = (*pLightParam->var.GetStruct())[1];
		MVariant& varValidPointLights = (*pLightParam->var.GetStruct())[4];
		{
			std::vector<MPointLight*> vActivePointLights(MPOINT_LIGHT_MAX_NUMBER);
			m_pScene->FindActivePointLights(pViewport->GetCamera()->GetWorldPosition(), vActivePointLights);
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

		MVariant& varSpotLights = (*pLightParam->var.GetStruct())[2];
		MVariant& varValidSpotLights = (*pLightParam->var.GetStruct())[5];
		{
			std::vector<MSpotLight*> vActiveSpotLights(MSPOT_LIGHT_MAX_NUMBER);
			m_pScene->FindActiveSpotLights(pViewport->GetCamera()->GetWorldPosition(), vActiveSpotLights);
			varValidSpotLights = (int)MSPOT_LIGHT_MAX_NUMBER;

			MVariantArray& vSpotLights = *varSpotLights.GetArray();
			for (unsigned int i = 0; i < vSpotLights.GetMemberCount(); ++i)
			{
				if (MSpotLight* pLight = vActiveSpotLights[i])
				{
					Vector3 f3SpotDirection = pLight->GetWorldDirection();
					f3SpotDirection.Normalize();
					MStruct& cSpotLight = *vSpotLights[i].GetStruct();
					cSpotLight[0] = pLight->GetWorldPosition();
					cSpotLight[1] = pLight->GetInnerCutOffRadius();
					cSpotLight[2] = f3SpotDirection;
					cSpotLight[3] = pLight->GetOuterCutOffRadius();
					cSpotLight[4] = pLight->GetDiffuseColor().ToVector3();
					cSpotLight[5] = pLight->GetSpecularColor().ToVector3();

				}
				else
				{
					varValidSpotLights = (int)i;
				}
			}
		}

		pLightParam->SetDirty();
		pRenderer->SetVertexShaderParam(*pLightParam);
		pLightParam->SetDirty();
		pRenderer->SetPixelShaderParam(*pLightParam);
	}
}

void MRenderSystem::DrawMeshInstance(MRenderInfo& info)
{
// 	MShaderParam* pMeshMatrixParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX);
// 	if (nullptr == pMeshMatrixParam)
// 		return;
// 
// 	MShaderParam* pAnimationParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_ANIMATION);
// 
// 	for (MaterialMeshInsGroup* pGroup : m_vMatMeshInsGroup)
// 	{
// 		MMaterial* pMaterial = pGroup->pMat;
// 		//使用材质
// 		if (!pRenderer->SetUseMaterial(pMaterial))
// 			continue;
// 
// 		//更新材质的纹理资源，最好移到Material的更新
// 		pRenderer->UpdateMaterialResource();
// 
// 		for (MIModelMeshInstance* pMeshIns : pGroup->vMeshIns)
// 		{
// 			if (!pMeshIns->GetVisibleRecursively())
// 				continue;
// 
// 			if (MCameraFrustum::EOUTSIDE == pViewport->GetCameraFrustum()->ContainTest(*pMeshIns->GetBoundsAABB()))
// 				continue;
// 
// 			Matrix4 worldTrans = pMeshIns->GetWorldTransform();
// 			//Transposed and Inverse.
// 			Matrix3 matNormal(worldTrans.Transposed().Inverse(), 3, 3);
// 
// 			MStruct& cStruct = *pMeshMatrixParam->var.GetStruct();
// 			cStruct[0] = worldTrans;
// 			cStruct[1] = matNormal;
// 
// 			pMeshMatrixParam->SetDirty();
// 			pRenderer->SetVertexShaderParam(*pMeshMatrixParam);
// 
// 			if (MModelInstance* pModel = dynamic_cast<MModelInstance*>(pMeshIns->GetParent()))
// 			{
// 				if (pAnimationParam && pModel->GetSkeleton())
// 				{
// 
// 					MStruct& cAnimationStruct = *pAnimationParam->var.GetStruct();
// 					MVariantArray& cBonesArray = *cAnimationStruct[0].GetArray();
// 
// 					const std::vector<MBone*>& bones = pModel->GetSkeleton()->GetAllBones();
// 					unsigned int size = bones.size();
// 					if (size > MBONES_MAX_NUMBER) size = MBONES_MAX_NUMBER;
// 
// 					for (unsigned int i = 0; i < size; ++i)
// 					{
// 						cBonesArray[i] = bones[i]->GetTransformInModelWorld();
// 					}
// 
// 					pAnimationParam->SetDirty();
// 					pRenderer->SetVertexShaderParam(*pAnimationParam);
// 				}
// 			}
// 
// 			pRenderer->UpdateMaterialParam();
// 			pRenderer->DrawMesh(pMeshIns->GetMesh(pMeshIns->GetDetailLevel()));
// 		}
// 	}
}

void MRenderSystem::DrawModelInstance(MRenderInfo& info)
{
	for (MModelInstance* pModelIns : *m_pScene->GetModelInstances())
	{
		if (pModelIns->GetDrawBoundingBox())
		{
			DrawBoundingBox(pRenderer, pViewport, pModelIns);
		}

		for (MNode* pChild : pModelIns->GetFixedChildren())
		{
			if (MIModelMeshInstance* pMeshIns = dynamic_cast<MIModelMeshInstance*>(pChild))
			{
				if (pMeshIns->GetDrawBoundingSphere())
				{
					DrawBoundingSphere(pRenderer, pViewport, pMeshIns);
				}
			}
		}
	}


}

void MRenderSystem::DrawSkyBox(MRenderInfo& info)
{
	pRenderer->SetRasterizerType(MIRenderer::MERasterizerType::ESolid | MIRenderer::MERasterizerType::ECullNone);

	MSkyBox* pSkyBox = m_pScene->GetSkyBox();

	if (pSkyBox)
	{
		if (MIMeshInstance* pMeshIns = pSkyBox->GetMeshInstance())
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

void MRenderSystem::DrawPainter(MRenderInfo& info)
{
	MTransformCoord3D* pTransformCoord = m_pScene->GetTransformCoord();
	pTransformCoord->Render(pRenderer, pViewport);
}

void MRenderSystem::DrawBoundingBox(MRenderInfo& info, MModelInstance* pModelIns)
{
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes;
	if (!pRenderer->SetUseMaterial(pMaterial))
		return;

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
		for (int i = 0; i < 2; ++i)
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

void MRenderSystem::DrawBoundingSphere(MRenderInfo& info, MIMeshInstance* pMeshIns)
{
	MResource* pSphereResource = m_pEngine->GetResourceManager()->LoadResource("./Model/Sphere.fbx");
	MMaterialResource* pStaticMeshMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_STATIC);

	MMaterial& mat = *pStaticMeshMaterialRes;
	unsigned int unType = MIRenderer::EWireframe | MIRenderer::ECullNone;
	mat.SetRenderState(unType);

	MShaderParam* pMeshMatrixParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX);
	if (nullptr == pMeshMatrixParam)
		return;

	if (!pRenderer->SetUseMaterial(pStaticMeshMaterialRes))
		return;

	MTransform trans;
	if (MBoundsSphere* pSphere = pMeshIns->GetBoundsSphere())
	{
		float fScale = pSphere->m_fRadius / 3.8f;
		trans.SetPosition(pSphere->m_v3CenterPoint);
		trans.SetScale(Vector3(fScale, fScale, fScale));
	}

	Matrix4 worldTrans = trans.GetMatrix();

	//Transposed and Inverse.
	Matrix3 matNormal(worldTrans.Transposed().Inverse(), 3, 3);

	MStruct& cStruct = *pMeshMatrixParam->var.GetStruct();
	cStruct[0] = worldTrans;
	cStruct[1] = matNormal;

	pMeshMatrixParam->SetDirty();
	pRenderer->SetVertexShaderParam(*pMeshMatrixParam);

	if (MModelResource* pModelResource = dynamic_cast<MModelResource*>(pSphereResource))
	{
		for (MModelMeshStruct* pMeshData : *pModelResource->GetMeshes())
		{
			pRenderer->DrawMesh(pMeshData->GetMesh());
		}
	}
}

void MRenderSystem::DrawCameraFrustum(MRenderInfo& info, MCamera* pCamera)
{
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes;
	if (!pRenderer->SetUseMaterial(pMaterial))
		return;

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
