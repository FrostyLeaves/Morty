#include "Component/MComponent.h"

#include "Scene/MEntity.h"
#include "Scene/MScene.h"
#include "System/MNotifyManager.h"

#include "Flatbuffer/MComponent_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MComponent, MTypeClass)

MComponent::MComponent()
    : MTypeClass()
    , m_id()
    , m_entityID()
    , m_scene(nullptr)
    , m_valid(false)
{}

MComponent::~MComponent() {}

void MComponent::Initialize(MScene* pScene, const MGuid& id)
{
    m_scene    = pScene;
    m_entityID = id;

    m_valid = true;
}

void                      MComponent::Release() { m_valid = false; }

flatbuffers::Offset<void> MComponent::Serialize(flatbuffers::FlatBufferBuilder& fbb)
{
    fbs::MComponentBuilder compBuilder(fbb);

    return compBuilder.Finish().Union();
}

void MComponent::Deserialize(const void* pBufferPointer)
{
    MORTY_UNUSED(pBufferPointer);
    //const fbs::MComponent* fbcomponent = reinterpret_cast<const fbs::MComponent*>(pBufferPointer);
}

void MComponent::Deserialize(flatbuffers::FlatBufferBuilder& fbb)
{
    MORTY_UNUSED(fbb);
    //const fbs::MComponent* fbcomponent = fbs::GetMComponent(fbb.GetCurrentBufferPointer());
}

void MComponent::PostDeserialize(const std::map<MGuid, MGuid>& tRedirectGuid)
{
    MORTY_UNUSED(tRedirectGuid);
}

void MComponent::SendComponentNotify(const char* notify)
{
    if (MScene* pScene = GetScene())
    {
        if (MNotifyManager* pNotifySystem = pScene->GetManager<MNotifyManager>())
        {
            pNotifySystem->SendNotify(notify, GetScene(), GetComponentID());
        }
    }
}

MEntity* MComponent::GetEntity() const
{
    if (m_scene) { return m_scene->GetEntity(m_entityID); }
    return nullptr;
}

MEngine* MComponent::GetEngine() const
{
    if (!m_scene) return nullptr;

    return m_scene->GetEngine();
}

bool MComponentID::operator==(const MComponentID& id) const
{
    return pComponentType == id.pComponentType && nIdx == id.nIdx;
}

bool MComponentID::operator==(const MType* pType) const
{
    return pComponentType == pType;
}

bool MComponentID::operator<(const MComponentID& id) const
{
    if (pComponentType < id.pComponentType) return true;
    else if (pComponentType > id.pComponentType)
        return false;

    if (nIdx < id.nIdx) return true;

    return false;
}

bool MComponentID::operator<(const MType* pType) const { return pComponentType < pType; }

bool MComponentID::IsValid() const { return pComponentType; }
