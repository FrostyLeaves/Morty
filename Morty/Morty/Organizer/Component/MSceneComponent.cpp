#include "MSceneComponent.h"

M_OBJECT_IMPLEMENT(MSceneComponent, MComponent)

#include "MNode.h"

MSceneComponent::MSceneComponent()
	: MComponent()
	, m_transform()
	, m_m4Transform(Matrix4::IdentityMatrix)
	, m_m4WorldTransform(Matrix4::IdentityMatrix)
	, m_m4WorldToLocalTransform(Matrix4::IdentityMatrix)
	, m_bLocalTransformDirty(true)
	, m_bWorldTransformDirty(true)
	, m_bWorldToLocalTransformDirty(true)
{

}

MSceneComponent::~MSceneComponent()
{

}

void MSceneComponent::Initialize()
{
	Super::Initialize();
}

void MSceneComponent::Release()
{
	Super::Release();
}

void MSceneComponent::SetPosition(const Vector3& pos)
{
	m_transform.SetPosition(pos);
	LocalTransformDirty();
}

void MSceneComponent::SetWorldPosition(const Vector3& pos)
{
	Vector3 localPos = GetWorldToLocalTransform() * pos;

	SetPosition(localPos);
}

Vector3 MSceneComponent::GetWorldPosition()
{
	return GetWorldTransform() * Vector3(0, 0, 0);
}

void MSceneComponent::SetWorldRotation(const Quaternion& quat)
{
	Quaternion localQuat = GetWorldToLocalTransform().GetRotation() * quat;

	SetRotation(localQuat);
}

Quaternion MSceneComponent::GetWorldRotation()
{
	return GetWorldTransform().GetRotation();
}

void MSceneComponent::SetRotation(const Quaternion& quat)
{
	m_transform.SetRotation(quat);
	LocalTransformDirty();
}

void MSceneComponent::SetScale(const Vector3& scale)
{
	m_transform.SetScale(scale);
	LocalTransformDirty();
}

void MSceneComponent::SetTransform(const MTransform& trans)
{
	m_transform = trans;
	LocalTransformDirty();
}

void MSceneComponent::SetWorldTransform(const MTransform& trans)
{

}

void MSceneComponent::LookAt(const Vector3& v3TargetWorldPos, Vector3 v3UpDir)
{
	v3UpDir.Normalize();
	Vector3 v3WorldPos = GetWorldPosition();

	Vector3 v3Forward = v3TargetWorldPos - v3WorldPos;
	v3Forward.Normalize();

	Vector3 v3RightDir = v3UpDir.CrossProduct(v3Forward);
	v3RightDir.Normalize();

	v3UpDir = v3Forward.CrossProduct(v3RightDir);
	v3UpDir.Normalize();

	Matrix4 matRotate(v3RightDir.x, v3UpDir.x, v3Forward.x, 0,
		v3RightDir.y, v3UpDir.y, v3Forward.y, 0,
		v3RightDir.z, v3UpDir.z, v3Forward.z, 0,
		0, 0, 0, 1);

	Quaternion quat = matRotate.GetRotation();
	quat.Normalize();
	m_transform.SetRotation(quat);
	LocalTransformDirty();
}


Matrix4 MSceneComponent::GetParentWorldTransform()
{
	if (m_bWorldTransformDirty)
		UpdateWorldTransform();

	return m_m4WorldTransform;
}

Matrix4 MSceneComponent::GetWorldToLocalTransform()
{
	if (m_bWorldToLocalTransformDirty)
	{
		m_bWorldToLocalTransformDirty = false;

		m_m4WorldToLocalTransform = GetParentWorldTransform().Inverse();
	}

	return m_m4WorldToLocalTransform;
}

Matrix4 MSceneComponent::GetWorldTransform()
{
	return GetParentWorldTransform() * GetLocalTransform();
}

Matrix4 MSceneComponent::GetLocalTransform()
{
	if (m_bLocalTransformDirty)
	{
		m_m4Transform = m_transform.GetMatrix();
		m_bLocalTransformDirty = false;
	}

	return m_m4Transform;
}

Vector3 MSceneComponent::GetWorldUp()
{
	return GetParentWorldTransform() * m_transform.GetUp();
}

Vector3 MSceneComponent::GetWorldForward()
{
	return GetParentWorldTransform() * m_transform.GetForward();
}

Vector3 MSceneComponent::GetWorldRight()
{
	return GetParentWorldTransform() * m_transform.GetRight();
}

void MSceneComponent::WriteToStruct(MStruct& srt)
{
	Super::WriteToStruct(srt);

	M_SERIALIZER_WRITE_BEGIN;

	M_SERIALIZER_WRITE_VALUE("Position", GetPosition);
	M_SERIALIZER_WRITE_VALUE("Scale", GetScale);
	M_SERIALIZER_WRITE_VALUE("Rotation", GetRotation);

	M_SERIALIZER_END;
}

void MSceneComponent::ReadFromStruct(const MStruct& srt)
{
	Super::ReadFromStruct(srt);

	M_SERIALIZER_READ_BEGIN;

	M_SERIALIZER_READ_VALUE("Position", SetPosition, Vector3);
	M_SERIALIZER_READ_VALUE("Scale", SetScale, Vector3);
	M_SERIALIZER_READ_VALUE("Rotation", SetRotation, Quaternion);

	M_SERIALIZER_END;
}

void MSceneComponent::WorldTransformDirty()
{
	m_bWorldTransformDirty = true;
	m_bWorldToLocalTransformDirty = true;

	SendComponentNotify("TransformDirty");
}

void MSceneComponent::LocalTransformDirty()
{
	if (m_bLocalTransformDirty)
		return;

	m_bLocalTransformDirty = true;

	SendComponentNotify("TransformDirty");

	if (MNode* pOwnerNode = GetOwnerNode())
	{
		WorldTransformDirtyRecursively(pOwnerNode);
	}


}

void MSceneComponent::UpdateWorldTransform()
{
	MNode* pOwnerNode = GetOwnerNode();
	if (!pOwnerNode)
		return;

	m_bWorldTransformDirty = false;

	m_m4WorldTransform = Matrix4::IdentityMatrix;

	MNode* pNode = pOwnerNode;
	while (pNode = pNode->GetParent())
	{
		if (MSceneComponent* pComponent = pNode->GetComponent<MSceneComponent>())
		{
			m_m4WorldTransform = pComponent->GetWorldTransform();
			return;
		}
	}
	
}

void MSceneComponent::WorldTransformDirtyRecursively(MNode* pNode)
{
	for (MNode* pChildNode : pNode->GetChildren())
	{
		if (MSceneComponent* pComponent = pChildNode->GetComponent<MSceneComponent>())
		{
			pComponent->WorldTransformDirty();
		}

		WorldTransformDirtyRecursively(pChildNode);
	}

	for (MNode* pChildNode : pNode->GetProtectedChildren())
	{
		if (MSceneComponent* pComponent = pChildNode->GetComponent<MSceneComponent>())
		{
			pComponent->WorldTransformDirty();
		}

		WorldTransformDirtyRecursively(pChildNode);
	}
}
