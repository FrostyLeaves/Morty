#include "Component/MRenderableMeshComponent.h"

MORTY_CLASS_IMPLEMENT(MRenderableMeshComponent, MComponent)

#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Utility/MFunction.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"


#include "Component/MSceneComponent.h"
#include "Component/MModelComponent.h"
#include "Component/MNotifyComponent.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Flatbuffer/MRenderableMeshComponent_generated.h"

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
	, m_unDetailLevel(MRenderGlobal::MESH_LOD_LEVEL_RANGE)

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
	
}

void MRenderableMeshComponent::Initialize()
{
	Super::Initialize();

	MEntity* pEntity = GetEntity();
	if (!pEntity)
	{
		GetEngine()->GetLogger()->Error("Component Initialize, OwnerNode == nullptr, Type: %s", GetTypeName().c_str());
		return;
	}

	if (MNotifyComponent* pNotifyComponent = pEntity->GetComponent<MNotifyComponent>())
	{
		pNotifyComponent->RegisterComponentNotify<MRenderableMeshComponent>(MString("TransformDirty"), M_CLASS_FUNCTION_BIND_0(MRenderableMeshComponent::OnTransformDirty, this));
		pNotifyComponent->RegisterComponentNotify<MRenderableMeshComponent>(MString("ParentChanged"), M_CLASS_FUNCTION_BIND_0(MRenderableMeshComponent::OnParentChanged, this));
	}
}

void MRenderableMeshComponent::Release()
{
	MEntity* pEntity = GetEntity();
	if (!pEntity)
	{
		GetEngine()->GetLogger()->Error("Component Release, OwnerNode == nullptr, Type: %s", GetTypeName().c_str());
		return;
	}

	if (MNotifyComponent* pNotifyComponent = pEntity->GetComponent<MNotifyComponent>())
	{
		pNotifyComponent->UnregisterComponentNotify<MRenderableMeshComponent>(MString("TransformDirty"));
		pNotifyComponent->UnregisterComponentNotify<MRenderableMeshComponent>(MString("ParentChanged"));
	}
	
	BindShaderParam(nullptr);

	Super::Release();
}

void MRenderableMeshComponent::SetMaterial(std::shared_ptr<MMaterial> pMaterial)
{
	if (m_Material.GetResource() == pMaterial)
		return;

	m_Material.SetResource(pMaterial);
	BindShaderParam(pMaterial);
}

std::shared_ptr<MMaterial> MRenderableMeshComponent::GetMaterial()
{
	return std::static_pointer_cast<MMaterial>(m_Material.GetResource());
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
	MEntity* pEntity = GetEntity();
	if (!pEntity)
		return;

	MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
	{
		GetEngine()->GetLogger()->Error("RenderableMeshComponent: SceneComponent Not Found!");
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
			Quaternion quat =  worldTrans.GetRotation();
			Matrix4 mat = quat;
			//Transposed and Inverse.
			Matrix3 matNormal(worldTrans, 3, 3);

			*m_pNormalMatrixParam = matNormal;
		}

		m_pTransformParam->SetDirty();
	}

	m_bTransformParamDirty = false;
}

bool MRenderableMeshComponent::SetMaterialPath(const MString& strPath)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strPath))
	{
		if (std::shared_ptr<MMaterialResource> pMaterialRes = MTypeClass::DynamicCast<MMaterialResource>(pResource))
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

void MRenderableMeshComponent::Load(std::shared_ptr<MResource> pResource)
{
	if (!pResource)
		return;

	if (std::shared_ptr<MMeshResource> pMeshResource = MTypeClass::DynamicCast<MMeshResource>(pResource))
	{
		m_Mesh.SetResource(pResource);

		if (m_Material.GetResource() == nullptr)
		{
			std::shared_ptr<MMaterial> pMaterial = MTypeClass::DynamicCast<MMaterial>(pMeshResource->GetDefaultMaterial());
			SetMaterial(pMaterial);
		}
	}
}

void MRenderableMeshComponent::SetMeshResourcePath(const MString& strResourcePath)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strResourcePath);
	Load(pResource);
}

MIMesh* MRenderableMeshComponent::GetMesh()
{
	std::shared_ptr<MMeshResource> pMeshResource = m_Mesh.GetResource<MMeshResource>();
	if (!pMeshResource)
		return nullptr;
	
	return pMeshResource->GetLevelMesh(GetDetailLevel());
}

MBoundsAABB* MRenderableMeshComponent::GetBoundsAABB()
{
	MEntity* pEntity = GetEntity();
	if (!pEntity)
		return &m_BoundsAABB;

	MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
		return &m_BoundsAABB;

	std::shared_ptr<MMeshResource> pMeshResource = m_Mesh.GetResource<MMeshResource>();
	if (!pMeshResource)
		return &m_BoundsAABB;

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
	MEntity* pEntity = GetEntity();
	if (!pEntity)
		return &m_BoundsSphere;

	MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
	if (!pSceneComponent)
		return &m_BoundsSphere;

	std::shared_ptr<MMeshResource> pMeshResource = m_Mesh.GetResource<MMeshResource>();
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

std::shared_ptr<MSkeletonInstance> MRenderableMeshComponent::GetSkeletonInstance()
{
	if (MModelComponent* pModelComponent = GetAttachedModelComponent())
		return pModelComponent->GetSkeleton();

	return nullptr;
}

MModelComponent* MRenderableMeshComponent::GetAttachedModelComponent()
{
	MEntity* pEntity = GetEntity();
	if (!pEntity)
		return nullptr;
	
	if (!m_bModelInstanceFound && !m_pModelComponent)
	{
		MSceneComponent* pSceneComponent = pEntity->GetComponent<MSceneComponent>();
		while (pSceneComponent)
		{
			pEntity = pSceneComponent->GetEntity();
			if (MModelComponent* pModelComponent = pEntity->GetComponent<MModelComponent>())
			{
				m_bModelInstanceFound = true;
				m_pModelComponent = pModelComponent;
				return m_pModelComponent;
			}

			pSceneComponent = GetScene()->GetComponent(pSceneComponent->GetParentComponent())->DynamicCast<MSceneComponent>();
		}
	}
	return m_pModelComponent;
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

flatbuffers::Offset<void> MRenderableMeshComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
	auto fb_super = Super::Serialize(fbb).o;
	auto&& fb_material_path = fbb.CreateString(GetMaterialPath());
	auto&& fb_mesh_path = fbb.CreateString(GetMeshResourcePath());
	mfbs::MRenderableMeshComponentBuilder builder(fbb);

	builder.add_gen_dir_shadow(GetGenerateDirLightShadow());
	builder.add_draw_bounding(GetDrawBoundingSphere());
	builder.add_lod((int)GetDetailLevel());
	builder.add_material_path(fb_material_path);
	builder.add_mesh_path(fb_mesh_path);
	builder.add_super(fb_super);


	return builder.Finish().Union();
}

void MRenderableMeshComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
	const mfbs::MRenderableMeshComponent* fbcomponent = mfbs::GetMRenderableMeshComponent(fbb.GetCurrentBufferPointer());
	Deserialize(fbcomponent);
}

void MRenderableMeshComponent::Deserialize(const void* pBufferPointer)
{
	const mfbs::MRenderableMeshComponent* pComponent = reinterpret_cast<const mfbs::MRenderableMeshComponent*>(pBufferPointer);

	Super::Deserialize(pComponent->super());

	SetGenerateDirLightShadow(pComponent->gen_dir_shadow());
	SetDrawBoundingSphere(pComponent->draw_bounding());
	SetDetailLevel(pComponent->lod());
	if (const flatbuffers::String* fb_mat_path = pComponent->material_path())
	{
		SetMaterialPath(fb_mat_path->c_str());
	}
	if (const flatbuffers::String* fb_mesh_path = pComponent->mesh_path())
	{
		SetMeshResourcePath(fb_mesh_path->c_str());
	}
}

void MRenderableMeshComponent::BindShaderParam(std::shared_ptr<MMaterial> pMaterial)
{
	if (m_pShaderParamSet)
	{
		MRenderSystem* pRenderSystem = GetEngine()->FindSystem<MRenderSystem>();
		m_pShaderParamSet->DestroyBuffer(pRenderSystem->GetDevice());
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
					m_pWorldMatrixParam = pSrt->GetValue<Matrix4>("U_matWorld");
					m_pNormalMatrixParam = pSrt->GetValue<Matrix3>("U_matNormal");
				}
			}
		}
	}
}
