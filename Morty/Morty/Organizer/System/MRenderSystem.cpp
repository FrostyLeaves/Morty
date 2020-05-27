#include "MRenderSystem.h"
#include "MEngine.h"
#include "MResourceManager.h"
#include "MIRenderer.h"

#include "MScene.h"
#include "MViewport.h"
#include "MTexture.h"
#include "MShadowTextureRenderTarget.h"
#include "MTransparentRenderTarget.h"

#include "MPainter.h"
#include "MTexture.h"
#include "MRenderStructure.h"
#include "MTransformCoord.h"

#include "MCamera.h"
#include "MSkyBox.h"
#include "MSkeleton.h"
#include "Model/MModelInstance.h"
#include "Model/MIMeshInstance.h"
#include "Model/MIModelMeshInstance.h"
#include "Light/MSpotLight.h"
#include "Light/MPointLight.h"
#include "Light/MDirectionalLight.h"

#include "MSkeleton.h"
#include "Material/MMaterialResource.h"
#include "Model/MModelResource.h"
#include "Model/MModelMeshStruct.h"

#include <algorithm>


M_OBJECT_IMPLEMENT(MRenderSystem, MISystem)


MRenderSystem::MRenderSystem()
	: MISystem()
	, m_cDepthPeelingMesh(true)
{
	MMesh<Vector2>& mesh = m_cDepthPeelingMesh;
	mesh.ResizeVertices(4);
	Vector2* vVertices = (Vector2*)mesh.GetVertices();

	vVertices[0] = Vector2(-1, -1);
	vVertices[1] = Vector2(1, -1);
	vVertices[2] = Vector2(-1, 1);
	vVertices[3] = Vector2(1, 1);

	mesh.ResizeIndices(2, 3);
	unsigned int* vIndices = mesh.GetIndices();

	vIndices[0] = 0;
	vIndices[1] = 2;
	vIndices[2] = 1;

	vIndices[3] = 2;
	vIndices[4] = 3;
	vIndices[5] = 1;
}

MRenderSystem::~MRenderSystem()
{
	m_cDepthPeelingMesh.DestroyBuffer(m_pEngine->GetDevice());
}

void MRenderSystem::Tick(const float& fDelta)
{

}

void MRenderSystem::Render(MIRenderer* pRenderer, MViewport* pViewport, MScene* pScene, MIRenderTarget* pRenderTarget)
{

	MRenderInfo info;

	info.pRenderTarget = pRenderTarget;
	info.pRenderer = pRenderer;
	info.pViewport = pViewport;
	info.pCamera = pViewport->GetCamera();
	info.pScene = pScene;

 	GenerateRenderGroup(info);
 	GenerateShadowMap(info);
 
 	Vector2 v2LeftTop = pViewport->GetLeftTop();
 	pRenderer->SetViewport(v2LeftTop.x, v2LeftTop.y, pViewport->GetWidth(), pViewport->GetHeight(), 0.0f, 1.0f);
 
 	UpdateShaderSharedParams(info);
	DrawPainter(info);
	DrawNormalMesh(info);
	DrawTransparentMesh(info);
	//	DrawSkyBox(info);
	DrawModelInstance(info);
}

void MRenderSystem::GenerateShadowMap(MRenderInfo& info)
{
	info.pDirectionalLight = info.pScene->FindActiveDirectionLight();
	if (nullptr == info.pDirectionalLight)
		return;

	Vector3 v3LightDir = info.pDirectionalLight->GetWorldDirection();

	Vector3 v3ShadowMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3ShadowMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);


	info.vShadowGroup.push_back(MShadowRenderGroup());

	std::vector<MModelInstance*>& vModels = *info.pScene->GetAllModelInstance();
	for (MModelInstance* pModelIns : vModels)
	{
		if (pModelIns->GetVisibleRecursively() && pModelIns->GetGenerateDirLightShadow())
		{
			MShadowRenderGroup* pGroup = nullptr;
			if (pModelIns->GetSkeleton())
			{
				info.vShadowGroup.push_back(MShadowRenderGroup());
				pGroup = &info.vShadowGroup.back();
				pGroup->pSkeletonInstance = pModelIns->GetSkeleton();
			}
			else
			{
				pGroup = &info.vShadowGroup.front();
			}
			MShadowRenderGroup& group = *pGroup;

			for (MNode* pChild : pModelIns->GetFixedChildren())
			{
				if (MIMeshInstance* pMeshIns = pChild->DynamicCast<MIMeshInstance>())
				{
					if (pMeshIns->GetVisible() && pMeshIns->GetGenerateDirLightShadow())
					{
						const MBoundsAABB* pBounds = pMeshIns->GetBoundsAABB();
						if (info.pViewport->GetCameraFrustum()->ContainTest(*pBounds, v3LightDir) != MCameraFrustum::EOUTSIDE)
						{
							group.vMeshInstances.push_back(pMeshIns);
							pBounds->UnionMinMax(v3ShadowMin, v3ShadowMax);
						}
					}
				}
			}
		}
	}

	info.cShadowRenderAABB.SetMinMax(v3ShadowMin, v3ShadowMax);

	info.m4DirLightInvProj = info.pViewport->GetLightInverseProjection(info.pDirectionalLight, info.cMeshRenderAABB, info.cShadowRenderAABB);

	MShadowTextureRenderTarget* pShadowRenderTarget = info.pScene->GetShadowRenderTarget();
	if (nullptr == pShadowRenderTarget || nullptr == info.pScene->FindActiveDirectionLight())
	{
		return;
	}
	//如果当前有Shader使用了ShadowMap，那么进行ShadowMap的更新
	if (SHADER_PARAM_CODE_SHADOW_MAP < MShaderBuffer::s_vTextureParams.size())
	{
		MShaderTextureParam* pShadowParam = MShaderBuffer::s_vTextureParams[SHADER_PARAM_CODE_SHADOW_MAP];
		if (pShadowParam && SHADER_PARAM_CODE_SHADOW_MAP == pShadowParam->unCode)
		{
			pShadowRenderTarget->Render(info.pRenderer, info.m4DirLightInvProj, &info.vShadowGroup);

			pShadowParam->pTexture = pShadowRenderTarget->m_pDepthTexture;
			info.pRenderer->SetPixelShaderTexture(*pShadowParam);
		}
	}

}

void MRenderSystem::UpdateShaderSharedParams(MRenderInfo& info)
{

	if (MShaderParam* pWorldMatrixParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_WORLD_MATRIX))
	{
		MStruct& cStruct = *pWorldMatrixParam->var.GetStruct();
		cStruct[0] = info.pViewport->GetCameraInverseProjection();
		cStruct[1] = info.m4DirLightInvProj;

		pWorldMatrixParam->SetDirty();
		info.pRenderer->SetVertexShaderParam(*pWorldMatrixParam);
	}

	if (MShaderParam* pWorldInfoParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_WORLDINFO))
	{
		if (info.pDirectionalLight)
		{
			(*pWorldInfoParam->var.GetStruct())[0] = info.pDirectionalLight->GetWorldDirection();
		}

		(*pWorldInfoParam->var.GetStruct())[1] = info.pViewport->GetCamera()->GetWorldPosition();

		(*pWorldInfoParam->var.GetStruct())[2] = info.pViewport->GetSize();

		pWorldInfoParam->SetDirty();
		info.pRenderer->SetVertexShaderParam(*pWorldInfoParam);
		pWorldInfoParam->SetDirty();
		info.pRenderer->SetPixelShaderParam(*pWorldInfoParam);
	}

	if (MShaderParam* pLightParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_LIGHT))
	{
		MVariant& varDirLightEnable = (*pLightParam->var.GetStruct())[3];
		if (info.pDirectionalLight)
		{
			varDirLightEnable = true;
			MVariant& varDirectionLight = (*pLightParam->var.GetStruct())[0];
			{
				MStruct& cLightStruct = *varDirectionLight.GetStruct();
				{
					cLightStruct[0] = info.pDirectionalLight->GetDiffuseColor().ToVector3();
					cLightStruct[1] = info.pDirectionalLight->GetSpecularColor().ToVector3();
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
			info.pScene->FindActivePointLights(info.pViewport->GetCamera()->GetWorldPosition(), vActivePointLights);
			varValidPointLights = 0;

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

					varValidPointLights = (int)i + 1;
				}
				else break;
			}
		}

		MVariant& varSpotLights = (*pLightParam->var.GetStruct())[2];
		MVariant& varValidSpotLights = (*pLightParam->var.GetStruct())[5];
		{
			std::vector<MSpotLight*> vActiveSpotLights(MSPOT_LIGHT_MAX_NUMBER);
			info.pScene->FindActiveSpotLights(info.pViewport->GetCamera()->GetWorldPosition(), vActiveSpotLights);
			varValidSpotLights = 0;

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

					varValidSpotLights = (int)i + 1;
				}
				else break;
			}
		}

		pLightParam->SetDirty();
		info.pRenderer->SetVertexShaderParam(*pLightParam);
		pLightParam->SetDirty();
		info.pRenderer->SetPixelShaderParam(*pLightParam);
	}
}

void MRenderSystem::DrawNormalMesh(MRenderInfo& info)
{
	MShaderParam* pMeshMatrixParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX);
	if (nullptr == pMeshMatrixParam)
		return;

	MShaderParam* pAnimationParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_ANIMATION);

	for (MMaterialGroup& group : info.vMaterialRenderGroup)
	{
		MMaterial* pMaterial = group.m_pMaterial;
		//使用材质
		if (!info.pRenderer->SetUseMaterial(pMaterial, true))
			continue;

		for (MIMeshInstance* pMeshIns : group.m_vMeshInstances)
		{
			DrawMeshInstance(info.pRenderer, pMeshIns, pMeshMatrixParam, pAnimationParam);
		}
	}
}

void MRenderSystem::DrawMeshInstance(MIRenderer*& pRenderer, MIMeshInstance*& pMeshInstance, MShaderParam*& pMeshMatrixParam, MShaderParam*& pAnimationParam)
{
	Matrix4 worldTrans = pMeshInstance->GetWorldTransform();
	//Transposed and Inverse.
	Matrix3 matNormal(worldTrans.Transposed().Inverse(), 3, 3);

	MStruct& cStruct = *pMeshMatrixParam->var.GetStruct();
	cStruct[0] = worldTrans;
	cStruct[1] = matNormal;

	pMeshMatrixParam->SetDirty();
	pRenderer->SetVertexShaderParam(*pMeshMatrixParam);

	if (pAnimationParam)
	{
		if (MModelInstance* pModel = pMeshInstance->GetParent()->DynamicCast<MModelInstance>())
		{
			if (pModel->GetSkeleton())
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
	}

	pRenderer->DrawMesh(pMeshInstance->GetMesh());
}

void MRenderSystem::DrawTransparentMesh(MRenderInfo& info)
{
	Vector2 v2Size = Vector2(info.pViewport->GetWidth(), info.pViewport->GetHeight());

 	MTransparentRenderTarget* pTransparentRenderTarget = info.pScene->GetTransparentRenderTarget();

	pTransparentRenderTarget->OnResize(v2Size.x, v2Size.y);
	pTransparentRenderTarget->Render(info.pRenderer, info.pRenderTarget, &info.vTransparentRenderGroup);

	MMaterialResource* pTextureMaterial = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DEPTH_PEELING);
	std::vector<MShaderTextureParam>& params = *pTextureMaterial->GetTextureParams();

	params[0].pTexture = pTransparentRenderTarget->GetBackTexture(0);
	params[1].pTexture = pTransparentRenderTarget->GetBackTexture(1);
	info.pRenderer->SetUseMaterial(pTextureMaterial);
	info.pRenderer->SetPixelShaderTexture(params[0]);
	info.pRenderer->SetPixelShaderTexture(params[1]);
	info.pRenderer->DrawMesh(&m_cDepthPeelingMesh);
}

void MRenderSystem::DrawModelInstance(MRenderInfo& info)
{
	for (MModelInstance* pModelIns : *info.pScene->GetAllModelInstance())
	{
		if (pModelIns->GetDrawBoundingBox())
		{
			DrawBoundingBox(info, pModelIns);
		}

		for (MNode* pChild : pModelIns->GetFixedChildren())
		{
			if (MIModelMeshInstance* pMeshIns = dynamic_cast<MIModelMeshInstance*>(pChild))
			{
				if (pMeshIns->GetDrawBoundingSphere())
				{
					DrawBoundingSphere(info, pMeshIns);
				}
			}
		}
	}


}

void MRenderSystem::DrawSkyBox(MRenderInfo& info)
{
	MSkyBox* pSkyBox = info.pScene->GetSkyBox();

	if (pSkyBox)
	{
		if (MIMeshInstance* pMeshIns = pSkyBox->GetMeshInstance())
		{
			MMaterial* pMaterial = pMeshIns->GetMaterial();

			if (MShaderParam* pMeshParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX))
			{
				MStruct& cStruct = *pMeshParam->var.GetStruct();
				Matrix4 mat(Matrix4::IdentityMatrix);
				Vector3 camPos = info.pViewport->GetCamera()->GetWorldPosition();
				mat.m[0][3] = camPos.x;
				mat.m[1][3] = camPos.y;
				mat.m[2][3] = camPos.z;
				cStruct[0] = mat;

				pMeshParam->SetDirty();
				info.pRenderer->SetVertexShaderParam(*pMeshParam);
			}

			if (info.pRenderer->SetUseMaterial(pMaterial, true))
			{
				info.pRenderer->DrawMesh(pMeshIns->GetMesh());
			}
		}

	}
}

void MRenderSystem::DrawPainter(MRenderInfo& info)
{
	MTransformCoord3D* pTransformCoord = info.pScene->GetTransformCoord();
	pTransformCoord->Render(info.pRenderer, info.pViewport);
}

void MRenderSystem::DrawBoundingBox(MRenderInfo& info, MModelInstance* pModelIns)
{
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes;
	if (!info.pRenderer->SetUseMaterial(pMaterial))
		return;

	info.pRenderer->UpdateMaterialParam();

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
			if (line.FillData(info.pViewport, meshs))
			{
				info.pRenderer->DrawMesh(&meshs);
				meshs.DestroyBuffer(m_pEngine->GetDevice());
			}
		}

		MPainter2DLine3D line(list[j], list[(j + 4)], MColor(1, 1, 1, 1), 1.0f);
		MMesh<MPainterVertex> meshs;
		if (line.FillData(info.pViewport, meshs))
		{
			info.pRenderer->DrawMesh(&meshs);
			meshs.DestroyBuffer(m_pEngine->GetDevice());
		}
	}
}

void MRenderSystem::DrawBoundingSphere(MRenderInfo& info, MIMeshInstance* pMeshIns)
{
	MResource* pSphereResource = m_pEngine->GetResourceManager()->LoadResource("./Model/Sphere.fbx");
	MMaterialResource* pStaticMeshMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_STATIC);

	MMaterial& mat = *pStaticMeshMaterialRes;
	mat.SetRasterizerType(MERasterizerType::ECullNone);

	MShaderParam* pMeshMatrixParam = MShaderBuffer::GetSharedParam(SHADER_PARAM_CODE_MESH_MATRIX);
	if (nullptr == pMeshMatrixParam)
		return;

	if (!info.pRenderer->SetUseMaterial(pStaticMeshMaterialRes))
		return;

	MTransform trans;
	if (MBoundsSphere* pSphere = pMeshIns->GetBoundsSphere())
	{
		float fScale = pSphere->m_fRadius / 3.8f;
		trans.SetPosition(pSphere->m_v3CenterPoint);
		trans.SetScale(Vector3(fScale, fScale, fScale));
	}

	Matrix4 worldTrans = trans.GetMatrix();
	MStruct& cStruct = *pMeshMatrixParam->var.GetStruct();
	cStruct[0] = worldTrans;

	pMeshMatrixParam->SetDirty();
	info.pRenderer->SetVertexShaderParam(*pMeshMatrixParam);

	if (MModelResource* pModelResource = dynamic_cast<MModelResource*>(pSphereResource))
	{
		for (MModelMeshStruct* pMeshData : *pModelResource->GetMeshes())
		{
			info.pRenderer->DrawMesh(pMeshData->GetMesh());
		}
	}
}

void MRenderSystem::DrawCameraFrustum(MRenderInfo& info, MCamera* pCamera)
{
	MMaterialResource* pDraw3DMaterialRes = m_pEngine->GetResourceManager()->LoadVirtualResource<MMaterialResource>(DEFAULT_MATERIAL_DRAW3D);
	MMaterial* pMaterial = pDraw3DMaterialRes;
	if (!info.pRenderer->SetUseMaterial(pMaterial))
		return;

	info.pRenderer->UpdateMaterialParam();


	Vector3 list[8];
	info.pViewport->GetCameraFrustum(list[0], list[1], list[2], list[3], list[4], list[5], list[6], list[7]);

	Vector2 begin, end;
	for (int j = 0; j < 4; ++j)
	{
		for (int i = 0; i < 2; ++i)
		{
			MPainter2DLine3D line(list[j + i * 4], list[(j + 1) % 4 + i * 4], MColor(i == 0 ? 0 : 1, 1, 1, 1), 1.0f);

			MMesh<MPainterVertex> meshs;
			if (line.FillData(info.pViewport, meshs))
			{
				info.pRenderer->DrawMesh(&meshs);
				meshs.DestroyBuffer(m_pEngine->GetDevice());
			}
		}

		MPainter2DLine3D line(list[j], list[(j + 4)], MColor(1, 1, 1, 1), 1.0f);
		MMesh<MPainterVertex> meshs;
		if (line.FillData(info.pViewport, meshs))
		{
			info.pRenderer->DrawMesh(&meshs);
			meshs.DestroyBuffer(m_pEngine->GetDevice());
		}
	}

}

void MRenderSystem::GenerateRenderGroup(MRenderInfo& info)
{
	Vector3 v3BoundsMin(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	Vector3 v3BoundsMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	std::vector<MMaterialGroup*>& matGroups = info.pScene->GetMaterialGroup();
	for (MMaterialGroup* pMaterialGroup : matGroups)
	{
		MMaterialGroup* pRenderGroup = nullptr;

		if (pMaterialGroup->m_pMaterial->GetMaterialType() == MEMaterialType::EDefault)
		{
			info.vMaterialRenderGroup.push_back(MMaterialGroup());
			pRenderGroup = &info.vMaterialRenderGroup.back();
		}
		else if (pMaterialGroup->m_pMaterial->GetMaterialType() == MEMaterialType::ETransparent)
		{
			info.vTransparentRenderGroup.push_back(MMaterialGroup());
			pRenderGroup = &info.vTransparentRenderGroup.back();
		}


		pRenderGroup->m_pMaterial = pMaterialGroup->m_pMaterial;

		for (MIMeshInstance* pMeshIns : pMaterialGroup->m_vMeshInstances)
		{
			if (!pMeshIns->GetVisibleRecursively())
				continue;

			if (MCameraFrustum::EOUTSIDE == info.pViewport->GetCameraFrustum()->ContainTest(*pMeshIns->GetBoundsAABB()))
				continue;

			pRenderGroup->m_vMeshInstances.push_back(pMeshIns);

			const MBoundsAABB* pBounds = pMeshIns->GetBoundsAABB();
			pBounds->UnionMinMax(v3BoundsMin, v3BoundsMax);
		}
	}

	info.cMeshRenderAABB.SetMinMax(v3BoundsMin, v3BoundsMax);
}
