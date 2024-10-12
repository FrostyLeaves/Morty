#include "Component/MSceneComponent.h"

#include "MTransform_generated.h"
#include "Math/MMath.h"
#include "Scene/MScene.h"
#include "Utility/MFunction.h"

#include "Module/MCoreNotify.h"

#include "Flatbuffer/MSceneComponent_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MSceneComponent, MComponent)

MSceneComponent::MSceneComponent()
    : MComponent()
    , m_transform()
    , m_transformMatrix(Matrix4::IdentityMatrix)
    , m_worldTransform(Matrix4::IdentityMatrix)
    , m_worldToLocalTransform(Matrix4::IdentityMatrix)
    , m_localTransformDirty(true)
    , m_worldTransformDirty(true)
    , m_worldToLocalTransformDirty(true)
    , m_visible(true)
    , m_visibleRecursively(true)

    , m_parentGuid(MGuid::invalid)
    , m_attachParent()
    , m_attachChildren()
{}

MSceneComponent::~MSceneComponent() = default;

bool MSceneComponent::GetVisibleRecursively() const { return m_visibleRecursively; }

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

void MSceneComponent::SetWorldScale(const Vector3 scale)
{
    Vector3 localScale = GetWorldToLocalTransform().GetScale();

    SetScale(
            Vector3(scale.x / localScale.x, scale.y / localScale.y, scale.z / localScale.z
            )
    );
}

Vector3 MSceneComponent::GetWorldScale() { return GetWorldTransform().GetScale(); }

void    MSceneComponent::SetRotation(const Quaternion& quat)
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
    if (m_attachParent == parent) { return; }

    if (MComponent* pParent = GetScene()->GetComponent(m_attachParent))
    {
        if (auto* pParentScene = pParent->template DynamicCast<MSceneComponent>())
        {
            pParentScene->RemoveChild(this);
        }
    }

    m_attachParent = parent;

    bool bParentValid = false;

    if (MComponent* pParent = GetScene()->GetComponent(m_attachParent))
    {
        if (MSceneComponent* pParentScene =
                    pParent->template DynamicCast<MSceneComponent>())
        {
            pParentScene->AddChild(this);
            SetVisibleRecursively(pParentScene->GetVisibleRecursively() && GetVisible());

            WorldTransformDirty();
            WorldTransformDirtyRecursively();

            bParentValid = true;
        }

        m_parentGuid = pParent->GetEntity()->GetID();
    }
    else { m_parentGuid = MGuid::invalid; }


    if (!bParentValid) { SetVisibleRecursively(GetVisible()); }

    SendComponentNotify(MCoreNotify::NOTIFY_PARENT_CHANGED);
}

void MSceneComponent::SetVisible(const bool& bVisible)
{
    m_visible = bVisible;

    MSceneComponent* pParentComponent = GetScene()
                                                ->GetComponent(GetParentComponent())
                                                ->template DynamicCast<MSceneComponent>();
    if (pParentComponent)
    {
        SetVisibleRecursively(pParentComponent->GetVisibleRecursively() & m_visible);
    }
    else { SetVisibleRecursively(m_visible); }
}

void MSceneComponent::SetVisibleRecursively(const bool& bVisible)
{
    if (m_visibleRecursively == bVisible) { return; }

    m_visibleRecursively = bVisible;

    SendComponentNotify(MCoreNotify::NOTIFY_VISIBLE_CHANGED);

    for (auto& child: m_attachChildren)
    {
        if (auto* pChildComponent = GetScene()
                                            ->GetComponent(child)
                                            ->template DynamicCast<MSceneComponent>())
        {
            pChildComponent->SetVisibleRecursively(
                    m_visibleRecursively & pChildComponent->GetVisible()
            );
        }
    }
}

void MSceneComponent::SetParent(MSceneComponent* pParent)
{
    if (pParent) { SetParentComponent(pParent->GetComponentID()); }
    else { SetParentComponent(MComponentID()); }
}

MSceneComponent* MSceneComponent::GetParent()
{
    if (MScene* pScene = GetScene())
    {
        if (MComponent* pComponent = pScene->GetComponent(GetParentComponent()))
        {
            return pComponent->template DynamicCast<MSceneComponent>();
        }
    }

    return nullptr;
}

void MSceneComponent::LookAt(const Vector3& v3TargetWorldPos, Vector3 v3UpDir)
{
    v3UpDir.Normalize();
    Vector3    v3WorldPos = GetWorldPosition();

    Vector3    v3Forward = v3TargetWorldPos - v3WorldPos;

    Matrix4    matRotate = MMath::LookAt(v3Forward, v3UpDir);

    Quaternion quat = matRotate.GetRotation();
    quat.Normalize();
    m_transform.SetRotation(quat);
    LocalTransformDirty();
}

Matrix4 MSceneComponent::GetParentWorldTransform()
{
    if (!m_attachParent.IsValid()) return Matrix4::IdentityMatrix;

    MComponent* pParent = GetScene()->GetComponent(m_attachParent);
    if (!pParent) return Matrix4::IdentityMatrix;

    auto* pParentScene = pParent->template DynamicCast<MSceneComponent>();
    if (!pParentScene) return Matrix4::IdentityMatrix;

    return pParentScene->GetWorldTransform();
}

Matrix4 MSceneComponent::GetWorldToLocalTransform()
{
    if (m_worldToLocalTransformDirty)
    {
        m_worldToLocalTransformDirty = false;

        m_worldToLocalTransform = GetParentWorldTransform().Inverse();
    }

    return m_worldToLocalTransform;
}

Matrix4 MSceneComponent::GetWorldTransform()
{
    if (m_worldTransformDirty)
    {
        m_worldTransformDirty = false;

        m_worldTransform = GetParentWorldTransform() * GetLocalTransform();
    }

    return m_worldTransform;
}

Matrix4 MSceneComponent::GetLocalTransform()
{
    if (m_localTransformDirty)
    {
        m_transformMatrix     = m_transform.GetMatrix();
        m_localTransformDirty = false;
    }

    return m_transformMatrix;
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

void MSceneComponent::CallRecursivelyFunction(
        MEntity*                      pEntity,
        std::function<void(MEntity*)> func
)
{
    if (!pEntity) return;

    MScene* pScene = pEntity->GetScene();
    if (!pScene) return;

    auto* pComponent = pEntity->GetComponent<MSceneComponent>();
    if (!pComponent) return;

    func(pEntity);

    const auto& children = pComponent->GetChildrenComponent();
    for (const auto& child: children)
    {
        if (MSceneComponent* pSceneComponent =
                    pScene->GetComponent(child)->template DynamicCast<MSceneComponent>())
        {
            CallRecursivelyFunction(pSceneComponent->GetEntity(), func);
        }
    }
}

flatbuffers::Offset<void> MSceneComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    flatbuffers::Offset<fbs::MComponent> fbSuper     = Super::Serialize(fbb).o;
    flatbuffers::Offset<fbs::MTransform> fbTransform = m_transform.Serialize(fbb).o;

    fbs::MSceneComponentBuilder          compBuilder(fbb);

    compBuilder.add_transform(fbTransform);

    if (MSceneComponent* pParent = GetParent())
    {
        if (MEntity* pParentEntity = pParent->GetEntity())
        {
            MGuid      id = pParentEntity->GetID();
            fbs::MGuid fbguid(id.data[0], id.data[1], id.data[2], id.data[3]);
            compBuilder.add_parent(&fbguid);
        }
    }

    compBuilder.add_super(fbSuper);

    return compBuilder.Finish().Union();
}

void MSceneComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
    const fbs::MSceneComponent* fbComponent =
            fbs::GetMSceneComponent(fbb.GetCurrentBufferPointer());
    Deserialize(fbComponent);
}

void MSceneComponent::Deserialize(const void* pBufferPointer)
{
    const auto* fbComponent =
            reinterpret_cast<const fbs::MSceneComponent*>(pBufferPointer);

    Super::Deserialize(fbComponent->super());


    MTransform transform;
    transform.Deserialize(fbComponent->transform());
    SetTransform(transform);

    if (const fbs::MGuid* fbguid = fbComponent->parent())
    {
        m_parentGuid =
                MGuid(fbguid->data0(), fbguid->data1(), fbguid->data2(), fbguid->data3());
    }
}

void MSceneComponent::PostDeserialize(const std::map<MGuid, MGuid>& tRedirectGuid)
{
    const MGuid redirectGuid = tRedirectGuid.at(m_parentGuid);

    if (MEntity* pEntity = GetScene()->GetEntity(redirectGuid))
    {
        SetParent(pEntity->GetComponent<MSceneComponent>());
    }
    else { MORTY_ASSERT(pEntity || redirectGuid == MGuid::invalid); }
}

void MSceneComponent::WorldTransformDirty()
{
    m_worldTransformDirty        = true;
    m_worldToLocalTransformDirty = true;

    SendComponentNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED);
}

void MSceneComponent::LocalTransformDirty()
{
    if (m_localTransformDirty) return;

    m_localTransformDirty        = true;
    m_worldTransformDirty        = true;
    m_worldToLocalTransformDirty = true;

    SendComponentNotify(MCoreNotify::NOTIFY_TRANSFORM_CHANGED);

    WorldTransformDirtyRecursively();
}

void MSceneComponent::WorldTransformDirtyRecursively()
{
    for (const MComponentID& id: m_attachChildren)
    {
        if (MComponent* pComponent = GetScene()->GetComponent(id))
        {
            if (MSceneComponent* pSceneComponent =
                        pComponent->template DynamicCast<MSceneComponent>())
            {
                pSceneComponent->WorldTransformDirty();
                pSceneComponent->WorldTransformDirtyRecursively();
            }
        }
    }
}

void MSceneComponent::AddChild(MSceneComponent* pChild)
{
    m_attachChildren.push_back(pChild->GetComponentID());
}

void MSceneComponent::RemoveChild(MSceneComponent* pChild)
{
    ERASE_FIRST_VECTOR(m_attachChildren, pChild->GetComponentID());
}
