#include "MRenderableMeshComponent.h"

M_OBJECT_IMPLEMENT(MRenderableMeshComponent, MComponent)

#include "MNode.h"
#include "MScene.h"
#include "MEngine.h"
#include "MMaterial.h"
#include "MFunction.h"

#include "MMeshResource.h"
#include "MSceneComponent.h"
#include "MModelComponent.h"
#include "MResourceManager.h"
#include "MMaterialResource.h"

MRenderableMeshComponent::MRenderableMeshComponent()
	: MComponent()
	, m_Mesh()
	, m_Material()
	, m_pShaderParamSet(nullptr)
	, m_pTransformParam(nullptr)
	, m_pWorldMatrixParam(nullptr)
	, m_pNormalMatrixParam(nullptr)
	, m_pModelComponent(nullptr)
	, m_BoundsAABB()
	, m_BoundsSphere()
	, m_eShadowType(MEShadowType::ENone)
	, m_unDetailLevel(MGlobal::MMESH_LOD_LEVEL_RANGE)

	, m_bDrawBoundingSphere(false)
	, m_bGenerateDirLightShadow(false)
	, m_bModelInstanceFound(false)
	, m_bTransformParamDirty(true)
	, m_bBoundsAABBDirty(true)
	, m_bBoundsSphereDirty(true)
{

}

MRenderableMeshComponent::~MRenderableMeshComponent()
{
	BindShaderParam(nullptr);
}

void MRenderableMeshComponent::Initialize()
{
	MNode* pOwnerNode = GetOwnerNode();
	if (!pOwnerNode)
	{
		MLogManager::GetInstance()->Error("Component Initialize, OwnerNode == nullptr, Type: %s", GetTypeName().c_str());
		return;
	}

	pOwnerNode->RegisterComponentNotify<MRenderableMeshComponent>(MString("TransformDirty"), M_CLASS_FUNCTION_BIND_0(MRenderableMeshComponent::OnTransformDirty));

	pOwnerNode->RegisterComponentNotify<MRenderableMeshComponent>(MString("ParentChanged"), M_CLASS_FUNCTION_BIND_0(MRenderableMeshComponent::OnParentChanged));
}

void MRenderableMeshComponent::Release()
{
	MNode* pOwnerNode = GetOwnerNode();
	if (!pOwnerNode)
	{
		MLogManager::GetInstance()->Error("Component Release, OwnerNode == nullptr, Type: %s", GetTypeName().c_str());
		return;
	}
	pOwnerNode->UnregisterComponentNotify<MRenderableMeshComponent>(MString("TransformDirty"));
}

void MRenderableMeshComponent::SetMaterial(MMaterial* pMaterial)
{
	if (m_Material.GetResource() == pMaterial)
		return;

	MScene* pScene = GetScene();

	if (pScene)
	{
		pScene->RemoveMaterialGroup(this);
	}

	m_Material.SetResource(pMaterial);
	BindShaderParam(pMaterial);

	if (pScene)
	{
		pScene->InsertMaterialGroup(this);
	}

}

MMaterial* MRenderableMeshComponent::GetMaterial()
{
	return static_cast<MMaterial*>(m_Material.GetResource());
}

MShaderParamSet* MRenderableMeshComponent::GetShaderMeshParamSet()
{
	if (m_bTransformParamDirty)
	{
		UpdateShaderMeshParam();
	}

	return m_pShaderParamSet;
}

void MRenderableMeshComponent::UpdateShaderMeshParam()
{
	MNode* pNode = GetOwnerNode();
	if (!pNode)
		return;

	MSceneComponent* pSceneComponent = pNode->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
	{
		MLogManager::GetInstance()->Error("RenderableMeshComponent: SceneComponent Not Found!");
		return;
	}

	if (m_pTransformParam)
	{
		Matrix4 worldTrans = pSceneComponent->GetWorldTransform();

		if (m_pWorldMatrixParam)
		{
			*m_pWorldMatrixParam = worldTrans;
		}

		if (m_pNormalMatrixParam)
		{
			//Transposed and Inverse.
			Matrix3 matNormal(worldTrans.Transposed().Inverse(), 3, 3);

			*m_pNormalMatrixParam = matNormal;
		}

		m_pTransformParam->SetDirty();
	}

	m_bTransformParamDirty = false;
}

bool MRenderableMeshComponent::SetMaterialPath(const MString& strPath)
{
	if (MResource* pResource = GetEngine()->GetResourceManager()->LoadResource(strPath))
	{
		if (MMaterialResource* pMaterialRes = pResource->DynamicCast<MMaterialResource>())
		{
			SetMaterial(pMaterialRes);
			return true;
		}
	}

	return false;
}

MString MRenderableMeshComponent::GetMaterialPath()
{
	return GetMaterial() ? GetMaterial()->GetResourcePath() : "";
}

void MRenderableMeshComponent::Load(MResource* pResource)
{
	if (!pResource)
		return;

	if (MMeshResource* pMeshResource = pResource->DynamicCast<MMeshResource>())
	{
		m_Mesh.SetResource(pResource);

		if (m_Material.GetResource() == nullptr)
		{
			MMaterial* pMaterial = pMeshResource->GetDefaultMaterial()->DynamicCast<MMaterial>();
			SetMaterial(pMaterial);
		}
	}
}

void MRenderableMeshComponent::SetMeshResourcePath(const MString& strResourcePath)
{
	MResource* pResource = GetEngine()->GetResourceManager()->LoadResource(strResourcePath);
	Load(pResource);
}

MIMesh* MRenderableMeshComponent::GetMesh()
{
	MMeshResource* pMeshResource = m_Mesh.GetResource<MMeshResource>();
	if (!pMeshResource)
		return nullptr;
	
	return pMeshResource->GetLevelMesh(GetDetailLevel());
}

MBoundsAABB* MRenderableMeshComponent::GetBoundsAABB()
{
	MNode* pNode = GetOwnerNode();
	if (!pNode)
		return &m_BoundsAABB;

	MSceneComponent* pSceneComponent = pNode->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
		return &m_BoundsAABB;

	MMeshResource* pMeshResource = m_Mesh.GetResource<MMeshResource>();
	if (!pMeshResource)
		return nullptr;

	if (m_bBoundsAABBDirty && pMeshResource)
	{
		Matrix4 matWorldTrans = pSceneComponent->GetWorldTransform();
		Vector3 v3Position = pSceneComponent->GetWorldPosition();
		m_BoundsAABB.SetBoundsOBB(v3Position, matWorldTrans, *pMeshResource->GetMeshesDefaultOBB());
		m_bBoundsAABBDirty = false;
	}

	return &m_BoundsAABB;
}

MBoundsSphere* MRenderableMeshComponent::GetBoundsSphere()
{
	MNode* pNode = GetOwnerNode();
	if (!pNode)
		return &m_BoundsSphere;

	MSceneComponent* pSceneComponent = pNode->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
		return &m_BoundsSphere;

	MMeshResource* pMeshResource = m_Mesh.GetResource<MMeshResource>();
	if (!pMeshResource)
		return nullptr;

	if (m_bBoundsSphereDirty)
	{
		m_BoundsSphere = *pMeshResource->GetMeshesDefaultSphere();

		m_BoundsSphere.m_v3CenterPoint = pSceneComponent->GetWorldTransform() * m_BoundsSphere.m_v3CenterPoint;
		Matrix4 matScale = MMath::GetScaleAndRotation(pSceneComponent->GetWorldTransform());

		Vector3 v3Scale = matScale * Vector3(1, 1, 1);

		float fMaxScale = v3Scale.x;
		if (fMaxScale < v3Scale.y) fMaxScale = v3Scale.y;
		if (fMaxScale < v3Scale.z) fMaxScale = v3Scale.z;

		m_BoundsSphere.m_fRadius *= fMaxScale;

		m_bBoundsSphereDirty = false;
	}

	return &m_BoundsSphere;
}

MSkeletonInstance* MRenderableMeshComponent::GetSkeletonInstance()
{
	if (MModelComponent* pModelComponent = GetAttachedModelComponent())
		return pModelComponent->GetSkeleton();

	return nullptr;
}

MModelComponent* MRenderableMeshComponent::GetAttachedModelComponent()
{
	MNode* pNode = GetOwnerNode();
	
	if (!m_bModelInstanceFound && !m_pModelComponent)
	{
		while (pNode)
		{
			if (MModelComponent* pModelComponent = pNode->GetComponent<MModelComponent>())
			{
				m_bModelInstanceFound = true;
				m_pModelComponent = pModelComponent;
				return m_pModelComponent;
			}

			pNode = pNode->GetParent();
		}
	}
	return nullptr;
}

void MRenderableMeshComponent::OnTransformDirty()
{
	m_bTransformParamDirty = true;
	m_bBoundsAABBDirty = true;
	m_bBoundsSphereDirty = true;
}

void MRenderableMeshComponent::OnParentChanged()
{
	m_pModelComponent = nullptr;
	m_bModelInstanceFound = false;
}

void MRenderableMeshComponent::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);

	M_SERIALIZER_WRITE_BEGIN;
	M_SERIALIZER_WRITE_VALUE("GenDirShadow", GetGenerateDirLightShadow);
	M_SERIALIZER_WRITE_VALUE("DrawBounding", GetDrawBoundingSphere);
	M_SERIALIZER_WRITE_VALUE("LOD", (int)GetDetailLevel);
	M_SERIALIZER_WRITE_VALUE("MaterialPath", GetMaterialPath);
	M_SERIALIZER_WRITE_VALUE("MeshPath", GetMeshResourcePath);
	M_SERIALIZER_END;
}

void MRenderableMeshComponent::ReadFromStruct(const MStruct& srt)
{
	Super::ReadFromStruct(srt);

	M_SERIALIZER_READ_BEGIN;
	M_SERIALIZER_READ_VALUE("GenDirShadow", SetGenerateDirLightShadow, Bool);
	M_SERIALIZER_READ_VALUE("DrawBounding", SetDrawBoundingSphere, Bool);
	M_SERIALIZER_READ_VALUE("LOD", SetDetailLevel, Int);
	M_SERIALIZER_READ_VALUE("MaterialPath", SetMaterialPath, String);
	M_SERIALIZER_READ_VALUE("MeshPath", SetMeshResourcePath, String);
	M_SERIALIZER_END;
}

void MRenderableMeshComponent::BindShaderParam(MMaterial* pMaterial)
{
	if (m_pShaderParamSet)
	{
		m_pShaderParamSet->DestroyBuffer(GetEngine()->GetDevice());
		delete m_pShaderParamSet;
		m_pShaderParamSet = nullptr;
		m_pTransformParam = nullptr;
		m_pWorldMatrixParam = nullptr;
		m_pNormalMatrixParam = nullptr;
	}

	if (pMaterial)
	{
		if (MShaderParamSet* pParamSet = pMaterial->GetMeshParamSet())
		{
			m_pShaderParamSet = pParamSet->Clone();

			if (m_pTransformParam = m_pShaderParamSet->FindConstantParam("_M_E_cbMeshMatrix"))
			{
				if (MStruct* pSrt = m_pTransformParam->var.GetStruct())
				{
					m_pWorldMatrixParam = pSrt->FindMember<Matrix4>("U_matWorld");
					m_pNormalMatrixParam = pSrt->FindMember<Matrix3>("U_matNormal");
				}
			}
		}
	}
}
