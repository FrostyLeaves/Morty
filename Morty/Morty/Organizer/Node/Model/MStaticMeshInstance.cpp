#include "MStaticMeshInstance.h"
#include "Model/MModelResource.h"
#include "MEngine.h"
#include "MIDevice.h"
#include "MScene.h"
#include "MMaterial.h"
#include "MMath.h"

#include "Model/MModelResource.h"
#include "Model/MMeshResource.h"
#include "MResourceManager.h"

#include "MBounds.h"

M_OBJECT_IMPLEMENT(MStaticMeshInstance, MIModelMeshInstance)

MStaticMeshInstance::MStaticMeshInstance()
	: MIModelMeshInstance()
	, m_pMesh(nullptr)
	, m_Mesh()
	, m_bBoundsAABBDirty(true)
	, m_bBoundsSphereDirty(true)
{

}

MStaticMeshInstance::~MStaticMeshInstance()
{
}

MBoundsAABB* MStaticMeshInstance::GetBoundsAABB()
{
	if (m_bBoundsAABBDirty)
	{
		Matrix4 matWorldTrans = GetWorldTransform();
		Vector3 v3Position = GetWorldPosition();
		m_BoundsAABB.SetBoundsOBB(v3Position, matWorldTrans, *m_pMesh->GetMeshesDefaultOBB());
		m_bBoundsAABBDirty = false;
	}

	return &m_BoundsAABB;
}

MBoundsSphere* MStaticMeshInstance::GetBoundsSphere()
{
	if (m_bBoundsSphereDirty)
	{
		m_BoundsSphere = *m_pMesh->GetMeshesDefaultSphere();

		m_BoundsSphere.m_v3CenterPoint = GetWorldTransform() * m_BoundsSphere.m_v3CenterPoint;
		Matrix4 matScale = MMath::GetScaleAndRotation(GetWorldTransform());

		Vector3 v3Scale = matScale * Vector3(1, 1, 1);

		float fMaxScale = v3Scale.x;
		if (fMaxScale < v3Scale.y) fMaxScale = v3Scale.y;
		if (fMaxScale < v3Scale.z) fMaxScale = v3Scale.z;

		m_BoundsSphere.m_fRadius *= fMaxScale;

		m_bBoundsSphereDirty = false;
	}

	return &m_BoundsSphere;
}

void MStaticMeshInstance::Load(MResource* pResource)
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

void MStaticMeshInstance::SetMeshResourcePath(const MString& strResourcePath)
{
	MResource* pResource = GetEngine()->GetResourceManager()->LoadResource(strResourcePath);
	Load(pResource);
}

MIMesh* MStaticMeshInstance::GetMesh(const uint32_t& unDetailLevel)
{
	if (unDetailLevel == MMESH_LOD_LEVEL_RANGE)
		return m_pMesh->GetMesh();
	else return m_pMesh->GetLevelMesh(unDetailLevel);
}

void MStaticMeshInstance::OnDelete()
{
	SetMaterial(nullptr);

	Super::OnDelete();
}


void MStaticMeshInstance::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);

	M_SERIALIZER_BEGIN(Write);
	M_SERIALIZER_WRITE_VALUE("MaterialPath", GetMaterialPath);
	M_SERIALIZER_WRITE_VALUE("MeshPath", GetMeshResourcePath);

	M_SERIALIZER_END;
}

void MStaticMeshInstance::ReadFromStruct(MStruct& srt)
{
	Super::ReadFromStruct(srt);


	M_SERIALIZER_BEGIN(Read);
	M_SERIALIZER_READ_VALUE("MaterialPath", SetMaterialPath, String);
	M_SERIALIZER_READ_VALUE("MeshPath", SetMeshResourcePath, String);

	M_SERIALIZER_END;
}


void MStaticMeshInstance::WorldTransformDirty()
{
	Super::WorldTransformDirty();

	m_bBoundsAABBDirty = true;
	m_bBoundsSphereDirty = true;
}

void MStaticMeshInstance::LocalTransformDirty()
{
	Super::LocalTransformDirty();

	m_bBoundsAABBDirty = true;
	m_bBoundsSphereDirty = true;
}
