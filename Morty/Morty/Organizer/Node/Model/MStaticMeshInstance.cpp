#include "MStaticMeshInstance.h"
#include "Model/MModelResource.h"
#include "MScene.h"
#include "MMaterial.h"
#include "MMath.h"

#include "Model/MModelResource.h"
#include "Model/MModelMeshStruct.h"
#include "MResourceManager.h"

#include "MBounds.h"

M_OBJECT_IMPLEMENT(MStaticMeshInstance, MIModelMeshInstance)

MStaticMeshInstance::MStaticMeshInstance()
	: MIModelMeshInstance()
	, m_pMesh(nullptr)
	, m_bBoundsAABBDirty(true)
	, m_bBoundsSphereDirty(true)
{

}

MStaticMeshInstance::~MStaticMeshInstance()
{
}

void MStaticMeshInstance::SetMaterial(MMaterial* pMaterial)
{
	if (m_Material.GetResource() == pMaterial)
		return;

	if (m_pScene)
		m_pScene->RemoveMaterialGroup(this);

	m_Material.SetResource(pMaterial);

	if (m_pScene)
		m_pScene->InsertMaterialGroup(this);

}
 
MMaterial* MStaticMeshInstance::GetMaterial()
{
	return dynamic_cast<MMaterial*>(m_Material.GetResource());
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

void MStaticMeshInstance::SetMeshData(MModelMeshStruct* pMeshData)
{
	m_pMesh = pMeshData;

	if (m_Material.GetResource() == nullptr)
	{
		MMaterial* pMaterial = dynamic_cast<MMaterial*>(m_pMesh->GetDefaultMaterial());
		SetMaterial(pMaterial);
	}
}

void MStaticMeshInstance::SetMeshData(const MString& strModelResourcePath, const int& nIndex)
{
	if (MModelResource* pModelRes = m_pEngine->GetResourceManager()->LoadResource(strModelResourcePath)->DynamicCast<MModelResource>())
	{
		m_pMesh = (*pModelRes->GetMeshes())[nIndex];

		if (m_Material.GetResource() == nullptr)
		{
			MMaterial* pMaterial = dynamic_cast<MMaterial*>(m_pMesh->GetDefaultMaterial());
			SetMaterial(pMaterial);
		}
	}
}

MIMesh* MStaticMeshInstance::GetMesh(const unsigned int& unDetailLevel)
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

	if (m_pMesh)
	{
		if (MModelResource* pModelRes = m_pMesh->GetModelResource())
		{
			pStruct->AppendMVariantMove("ModelResource", MVariant(pModelRes->GetResourcePath()));
			pStruct->AppendMVariantMove("MeshIndex", MVariant((int)m_pMesh->GetMeshIndex()));
		}
	}
	
	M_SERIALIZER_END;
}

void MStaticMeshInstance::ReadFromStruct(MStruct& srt)
{
	Super::ReadFromStruct(srt);


	M_SERIALIZER_BEGIN(Read);
	M_SERIALIZER_READ_VALUE("MaterialPath", SetMaterialPath, String);
	
	
	if (MString* pModelResource = FindReadVariant<MString>(*pStruct, "ModelResource"))
	{
		if (int* pMeshIndex = FindReadVariant<int>(*pStruct, "MeshIndex"))
		{
			SetMeshData(*pModelResource, *pMeshIndex);
		}
	}



	M_SERIALIZER_END;
}

void MStaticMeshInstance::WorldTransformDirty()
{
	MIMeshInstance::WorldTransformDirty();

	m_bBoundsAABBDirty = true;
	m_bBoundsSphereDirty = true;
}

void MStaticMeshInstance::LocalTransformDirty()
{
	MIMeshInstance::LocalTransformDirty();

	m_bBoundsAABBDirty = true;
	m_bBoundsSphereDirty = true;
}
