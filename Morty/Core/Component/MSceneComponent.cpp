#include "Component/MSceneComponent.h"

#include "Math/MMath.h"
#include "Scene/MScene.h"
#include "Utility/MFunction.h"

#include "Flatbuffer/MSceneComponent_generated.h"

MORTY_CLASS_IMPLEMENT(MSceneComponent, MComponent)

MSceneComponent::MSceneComponent()
	: MComponent()
	, m_transform()
	, m_m4Transform(Matrix4::IdentityMatrix)
	, m_m4WorldTransform(Matrix4::IdentityMatrix)
	, m_m4WorldToLocalTransform(Matrix4::IdentityMatrix)
	, m_bLocalTransformDirty(true)
	, m_bWorldTransformDirty(true)
	, m_bWorldToLocalTransformDirty(true)
	, m_bVisible(true)
	, m_bVisibleRecursively(true)

	, m_parentGuid(MGuid::invalid)
	, m_attachParent()
	, m_vAttachChildren()
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

bool MSceneComponent::GetVisibleRecursively()
{
	return m_bVisibleRecursively;
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

void MSceneComponent::SetParentComponent(const MComponentID& parent)
{
	if (MComponent* pParent = GetScene()->GetComponent(m_attachParent))
	{
		if (MSceneComponent* pParentScene = pParent->DynamicCast<MSceneComponent>())
		{
			pParentScene->RemoveChild(this);
		}
	}
	
	m_attachParent = parent;

	bool bParentValid = false;

	if(MComponent* pParent = GetScene()->GetComponent(m_attachParent))
	{
		if (MSceneComponent* pParentScene = pParent->DynamicCast<MSceneComponent>())
		{
			pParentScene->AddChild(this);
			SetVisibleRecursively(pParentScene->GetVisibleRecursively() & GetVisible());
			bParentValid = true;
		}

		m_parentGuid = pParent->GetEntity()->GetID();
	}
	else
	{
		m_parentGuid = MGuid::invalid;
	}


	if(!bParentValid)
	{
		SetVisibleRecursively(GetVisible());
	}
}

void MSceneComponent::SetVisible(const bool& bVisible)
{
	m_bVisible = bVisible;

	MSceneComponent* pParentComponent = GetScene()->GetComponent(GetParentComponent())->DynamicCast<MSceneComponent>();
	if(pParentComponent)
		SetVisibleRecursively(pParentComponent->GetVisibleRecursively() & m_bVisible);
	else
		SetVisibleRecursively(m_bVisible);
}

void MSceneComponent::SetVisibleRecursively(const bool& bVisible)
{
	m_bVisibleRecursively = bVisible;

	for (auto& child : m_vAttachChildren)
	{
		if (MSceneComponent* pChildComponent = GetScene()->GetComponent(child)->DynamicCast<MSceneComponent>())
		{
			pChildComponent->SetVisibleRecursively(m_bVisibleRecursively & pChildComponent->GetVisible());
		}
	}
}

void MSceneComponent::SetParent(MSceneComponent* pParent)
{
	if (pParent)
	{
		SetParentComponent(pParent->GetComponentID());
	}
	else
	{
		SetParentComponent(MComponentID());
	}
}

MSceneComponent* MSceneComponent::GetParent()
{
	if (MScene* pScene = GetScene())
	{
		if (MComponent* pComponent = pScene->GetComponent(GetParentComponent()))
		{
			return pComponent->DynamicCast<MSceneComponent>();
		}
	}

	return nullptr;
}

void MSceneComponent::LookAt(const Vector3& v3TargetWorldPos, Vector3 v3UpDir)
{
	v3UpDir.Normalize();
	Vector3 v3WorldPos = GetWorldPosition();

	Vector3 v3Forward = v3TargetWorldPos - v3WorldPos;

	Matrix4 matRotate = MMath::LookAt(v3Forward, v3UpDir);

	Quaternion quat = matRotate.GetRotation();
	quat.Normalize();
	m_transform.SetRotation(quat);
	LocalTransformDirty();
}

Matrix4 MSceneComponent::GetParentWorldTransform()
{
	if (!m_attachParent.IsValid())
		return Matrix4::IdentityMatrix;

	MComponent* pParent = GetScene()->GetComponent(m_attachParent);
	if (!pParent)
		return Matrix4::IdentityMatrix;

	MSceneComponent* pParentScene = pParent->DynamicCast<MSceneComponent>();
	if (!pParentScene)
		return Matrix4::IdentityMatrix;

	return pParentScene->GetWorldTransform();
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
	if (m_bWorldTransformDirty)
	{
		m_bWorldTransformDirty = false;

		m_m4WorldTransform = GetParentWorldTransform() * GetLocalTransform();
	}

	return m_m4WorldTransform;
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

void MSceneComponent::CallRecursivelyFunction(MEntity* pEntity, std::function<void(MEntity*)> func)
{
	if (!pEntity)
		return;

	MScene* pScene = pEntity->GetScene();
	if (!pScene)
		return;

	MSceneComponent* pComponent = pEntity->GetComponent<MSceneComponent>();
	if (!pComponent)
		return;

	func(pEntity);

	const auto& children = pComponent->GetChildrenComponent();
	for (const auto& child : children)
	{
		if (MSceneComponent* pSceneComponent = pScene->GetComponent(child)->DynamicCast<MSceneComponent>())
		{
			CallRecursivelyFunction(pSceneComponent->GetEntity(), func);
		}
	}
}

flatbuffers::Offset<void> MSceneComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
	flatbuffers::Offset<void> fbsuper = Super::Serialize(fbb);

	mfbs::MSceneComponentBuilder compBuilder(fbb);

	Vector3 positon = GetPosition();
	Vector3 scale = GetScale();
	Quaternion rotation = GetRotation();
	compBuilder.add_position(reinterpret_cast<mfbs::Vector3*>(&positon));
	compBuilder.add_scale(reinterpret_cast<mfbs::Vector3*>(&scale));
	compBuilder.add_rotation(reinterpret_cast<mfbs::Quaternion*>(&rotation));

	if (MSceneComponent* pParent = GetParent())
	{
		if (MEntity* pParentEntity = pParent->GetEntity())
		{
			MGuid id = pParentEntity->GetID();
			mfbs::MGuid fbguid(id.data[0], id.data[1], id.data[2], id.data[3]);
			compBuilder.add_parent(&fbguid);
		}
	}

	compBuilder.add_super(flatbuffers::Offset<mfbs::MComponent>(fbsuper.o));

	return compBuilder.Finish().Union();
}

void MSceneComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
	const mfbs::MSceneComponent* fbcomponent = mfbs::GetMSceneComponent(fbb.GetCurrentBufferPointer());
	Deserialize(fbcomponent);
}

void MSceneComponent::Deserialize(const void* pBufferPointer)
{
	const mfbs::MSceneComponent* fbComponent = reinterpret_cast<const mfbs::MSceneComponent*>(pBufferPointer);

	Super::Deserialize(fbComponent->super());

	SetPosition(*reinterpret_cast<const Vector3*>(fbComponent->position()));
	SetScale(*reinterpret_cast<const Vector3*>(fbComponent->scale()));
	SetRotation(*reinterpret_cast<const Quaternion*>(fbComponent->rotation()));

	if (const mfbs::MGuid* fbguid = fbComponent->parent())
	{
		m_parentGuid = MGuid(fbguid->data0(), fbguid->data1(), fbguid->data2(), fbguid->data3());
	}
}

void MSceneComponent::PostDeserialize()
{
	if (MEntity* pEntity = GetScene()->GetEntity(m_parentGuid))
	{
		SetParent(pEntity->GetComponent<MSceneComponent>());
	}
	else
	{
		assert(pEntity || m_parentGuid == MGuid::invalid);
	}
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
	m_bWorldTransformDirty = true;
	m_bWorldToLocalTransformDirty = true;

	SendComponentNotify("TransformDirty");

	WorldTransformDirtyRecursively();
}

void MSceneComponent::WorldTransformDirtyRecursively()
{
	for (const MComponentID& id : m_vAttachChildren)
	{
		if (MComponent* pComponent = GetScene()->GetComponent(id))
		{
			if (MSceneComponent* pSceneComponent = pComponent->DynamicCast<MSceneComponent>())
			{
				pSceneComponent->WorldTransformDirty();
				pSceneComponent->WorldTransformDirtyRecursively();
			}
		}
	}
}

void MSceneComponent::AddChild(MSceneComponent* pChild)
{
	m_vAttachChildren.push_back(pChild->GetComponentID());
}

void MSceneComponent::RemoveChild(MSceneComponent* pChild)
{
	ERASE_FIRST_VECTOR(m_vAttachChildren, pChild->GetComponentID());
}
