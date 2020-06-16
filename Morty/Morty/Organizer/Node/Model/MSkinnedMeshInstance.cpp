#include "MSkinnedMeshInstance.h"
#include "MScene.h"
#include "MMaterial.h"
#include "Model/MModelInstance.h"
#include "Model/MMeshResource.h"
#include "Model/MModelResource.h"
#include "MResourceManager.h"

#include "MBounds.h"

M_OBJECT_IMPLEMENT(MSkinnedMeshInstance, MIModelMeshInstance)

MSkinnedMeshInstance::MSkinnedMeshInstance()
	: MIModelMeshInstance()
	, m_pMesh(nullptr)
	, m_Mesh()
	, m_Material()
	, m_bBoundsAABBDirty(true)
	, m_bBoundsSphereDirty(true)
{
	
}

MSkinnedMeshInstance::~MSkinnedMeshInstance()
{

}

void MSkinnedMeshInstance::SetMaterial(MMaterial* pMaterial)
{
	m_Material.SetResource(pMaterial);
}

MMaterial* MSkinnedMeshInstance::GetMaterial()
{
	return dynamic_cast<MMaterial*>(m_Material.GetResource());
}

MBoundsAABB* MSkinnedMeshInstance::GetBoundsAABB()
{
	//TODO get the Bounding Box by skeleton instance.

	if (m_bBoundsAABBDirty)
	{
		Matrix4 matWorldTrans = GetWorldTransform();
		Vector3 v3Position = GetWorldPosition();
		m_BoundsAABB.SetBoundsOBB(v3Position, matWorldTrans, *m_pMesh->GetMeshesDefaultOBB());
		m_bBoundsAABBDirty = false;
	}

	return &m_BoundsAABB;
}

MBoundsSphere* MSkinnedMeshInstance::GetBoundsSphere()
{
	if (m_bBoundsSphereDirty)
	{
		m_BoundsSphere = *m_pMesh->GetMeshesDefaultSphere();

		m_BoundsSphere.m_v3CenterPoint = GetWorldPosition();

		Vector3 v3Scale = GetScale();
		float fMaxScale = v3Scale.x;
		if (fMaxScale < v3Scale.y) fMaxScale = v3Scale.y;
		if (fMaxScale < v3Scale.z) fMaxScale = v3Scale.z;

		m_BoundsSphere.m_fRadius *= fMaxScale;

		m_bBoundsSphereDirty = false;
	}

	return &m_BoundsSphere;
}

void MSkinnedMeshInstance::Load(MResource* pResource)
{
	if (MMeshResource* pMeshResource = pResource->DynamicCast<MMeshResource>())
	{
		m_pMesh = pMeshResource;
		m_Mesh.SetResource(pResource);

		if (m_Material.GetResource() == nullptr)
		{
			MMaterial* pMaterial = m_pMesh->GetDefaultMaterial()->DynamicCast<MMaterial>();
			SetMaterial(pMaterial);
		}
	}
}

void MSkinnedMeshInstance::SetMeshResourcePath(const MString& strResourcePath)
{
	MResource* pResource = GetEngine()->GetResourceManager()->LoadResource(strResourcePath);
	Load(pResource);
}

void MSkinnedMeshInstance::SetMeshData(const MString& strModelResourcePath, const int& nIndex)
{
	if (MModelResource* pModelRes = m_pEngine->GetResourceManager()->LoadResource(strModelResourcePath)->DynamicCast<MModelResource>())
	{
		m_pMesh = pModelRes->GetMeshResources()[nIndex];

		if (m_Material.GetResource() == nullptr)
		{
			MMaterial* pMaterial = dynamic_cast<MMaterial*>(m_pMesh->GetDefaultMaterial());
			SetMaterial(pMaterial);
		}
	}
}

MIMesh* MSkinnedMeshInstance::GetMesh(const unsigned int& unDetailLevel)
{
	if (unDetailLevel == MMESH_LOD_LEVEL_RANGE)
		return m_pMesh->GetMesh();
	else return m_pMesh->GetLevelMesh(unDetailLevel);
}

MSkeletonInstance* MSkinnedMeshInstance::GetSkeletonInstance()
{
	if (MModelInstance* pModelIns = GetParent()->DynamicCast<MModelInstance>())
		return pModelIns->GetSkeleton();

	return nullptr;
}

void MSkinnedMeshInstance::OnDelete()
{
	SetMaterial(nullptr);

	Super::OnDelete();
}

void MSkinnedMeshInstance::UpdateSkeletonBoundsOBB()
{
	
}

void MSkinnedMeshInstance::WorldTransformDirty()
{
	MIMeshInstance::WorldTransformDirty();

	m_bBoundsAABBDirty = true;
}

void MSkinnedMeshInstance::LocalTransformDirty()
{
	MIMeshInstance::LocalTransformDirty();

	m_bBoundsAABBDirty = true;
}

void MSkinnedMeshInstance::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);

	M_SERIALIZER_BEGIN(Write);
	M_SERIALIZER_WRITE_VALUE("MaterialPath", GetMaterialPath);
	M_SERIALIZER_WRITE_VALUE("MeshPath", GetMeshResourcePath);
	
	M_SERIALIZER_END;
}

void MSkinnedMeshInstance::ReadFromStruct(MStruct& srt)
{
	Super::ReadFromStruct(srt);


	M_SERIALIZER_BEGIN(Read);
	M_SERIALIZER_READ_VALUE("MaterialPath", SetMaterialPath, String);
	M_SERIALIZER_READ_VALUE("MeshPath", SetMeshResourcePath, String);

	M_SERIALIZER_END;
}
