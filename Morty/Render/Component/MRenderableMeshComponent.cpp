#include "Component/MRenderableMeshComponent.h"

MORTY_CLASS_IMPLEMENT(MRenderableMeshComponent, MComponent)

#include "MRenderNotify.h"
#include "Scene/MScene.h"
#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Material/MMaterial.h"
#include "Utility/MFunction.h"

#include "Resource/MMeshResource.h"
#include "Resource/MMaterialResource.h"


#include "Component/MSceneComponent.h"
#include "Component/MModelComponent.h"

#include "System/MRenderSystem.h"
#include "System/MResourceSystem.h"

#include "Flatbuffer/MRenderableMeshComponent_generated.h"

MRenderableMeshComponent::MRenderableMeshComponent()
	: MComponent()
	, m_Mesh()
	, m_Material()
	, m_pModelComponent(nullptr)
	, m_BoundsAABB()
	, m_BoundsSphere()
	, m_eShadowType(MEShadowType::ENone)
	, m_unDetailLevel(MRenderGlobal::MESH_LOD_LEVEL_RANGE)

	, m_bDrawBoundingSphere(false)
	, m_bModelInstanceFound(false)
	, m_bTransformParamDirty(true)
	, m_bBoundsAABBDirty(true)
	, m_bBoundsSphereDirty(true)
{

}

MRenderableMeshComponent::~MRenderableMeshComponent()
{
	
}

void MRenderableMeshComponent::Release()
{
	Super::Release();
}

void MRenderableMeshComponent::SetMaterial(std::shared_ptr<MMaterialResource> pMaterial)
{
	if (m_Material.GetResource() == pMaterial)
		return;

	m_Material = pMaterial;
	SendComponentNotify(MRenderNotify::NOTIFY_MATERIAL_CHANGED);
}

std::shared_ptr<MMaterialResource> MRenderableMeshComponent::GetMaterialResource() const
{
	return m_Material.GetResource<MMaterialResource>();
}

std::shared_ptr<MMaterial> MRenderableMeshComponent::GetMaterial()
{
	if (auto pMaterialResource = m_Material.GetResource<MMaterialResource>())
	{
		return pMaterialResource->GetMaterial();
	}

	return nullptr;
}

bool MRenderableMeshComponent::SetMaterialPath(const MString& strPath)
{
	MResourceSystem* pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();
	if (std::shared_ptr<MResource> pResource = pResourceSystem->LoadResource(strPath))
	{
		if (std::shared_ptr<MMaterialResource> pMaterialResource = MTypeClass::DynamicCast<MMaterialResource>(pResource))
		{
			SetMaterial(pMaterialResource);
			return true;
		}
	}

	return false;
}

void MRenderableMeshComponent::Load(std::shared_ptr<MResource> pResource)
{
	if (!pResource)
		return;

	if (std::shared_ptr<MMeshResource> pMeshResource = MTypeClass::DynamicCast<MMeshResource>(pResource))
	{
		m_Mesh.SetResource(pResource);
		SendComponentNotify(MRenderNotify::NOTIFY_MESH_CHANGED);
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
	
	return pMeshResource->GetMesh();
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

MSkeletonInstance* MRenderableMeshComponent::GetSkeletonInstance()
{
	if (MModelComponent* pModelComponent = GetAttachedModelComponent())
		return pModelComponent->GetSkeleton();

	return nullptr;
}

void MRenderableMeshComponent::SetGenerateDirLightShadow(const bool& bGenerate)
{
	if (m_bGenerateDirLightShadow != bGenerate)
	{
		m_bGenerateDirLightShadow = bGenerate;
		SendComponentNotify(MRenderNotify::NOTIFY_GENERATE_SHADOW_CHANGED);
	}
}

void MRenderableMeshComponent::SetSceneCullEnable(bool bEnable)
{
	m_bSceneCullEnable = bEnable;
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
	auto fb_material = m_Material.Serialize(fbb).o;
	auto fb_mesh = m_Mesh.Serialize(fbb).o;
	mfbs::MRenderableMeshComponentBuilder builder(fbb);

	builder.add_gen_dir_shadow(GetGenerateDirLightShadow());
	builder.add_draw_bounding(GetDrawBoundingSphere());
	builder.add_lod((int)GetDetailLevel());
	builder.add_material(fb_material);
	builder.add_mesh(fb_mesh);
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
	auto pResourceSystem = GetEngine()->FindSystem<MResourceSystem>();

	const mfbs::MRenderableMeshComponent* pComponent = reinterpret_cast<const mfbs::MRenderableMeshComponent*>(pBufferPointer);

	Super::Deserialize(pComponent->super());

	SetGenerateDirLightShadow(pComponent->gen_dir_shadow());
	SetDrawBoundingSphere(pComponent->draw_bounding());
	SetDetailLevel(pComponent->lod());

	MResourceRef material;
	material.Deserialize(pResourceSystem, pComponent->material());
	SetMaterial(material.GetResource<MMaterialResource>());

	MResourceRef mesh;
	mesh.Deserialize(pResourceSystem, pComponent->mesh());
	Load(mesh.GetResource());

	OnTransformDirty();
	OnParentChanged();
}
