#include "Scene/MEntity.h"
#include "Engine/MEngine.h"
#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "Utility/MFileHelper.h"
#include "Variant/MVariant.h"

#include "Component/MComponent.h"
#include "Scene/MScene.h"

#include "Utility/MFunction.h"

#include "Flatbuffer/MEntity_generated.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MEntity, MTypeClass)

MEntity::MEntity()
    : MTypeClass()
    , m_scene(nullptr)
    , m_id()
    , m_strName("")
{}

MEntity::MEntity(MScene* pScene, const MGuid& nID)
    : MTypeClass()
    , m_scene(pScene)
    , m_id(nID)
    , m_strName("")
{}

MEntity::~MEntity() {}

MComponent* MEntity::RegisterComponent(const MType* pComponentType)
{
    if (!m_scene) { return nullptr; }

    return m_scene->AddComponent(this, pComponentType);
}

void MEntity::UnregisterComponent(const MType* pComponentType)
{
    if (!m_scene) return;

    m_scene->RemoveComponent(this, pComponentType);
}

void MEntity::UnregisterAllComponent()
{
    auto components = m_components;

    for (auto& pr: components) { UnregisterComponent(pr.first); }
}

bool MEntity::HasComponent(const MType* pComponentType)
{
    if (!m_scene) return false;

    return m_components.find(pComponentType) != m_components.end();
}

MComponent* MEntity::GetComponent(const MType* pComponentType)
{
    if (!m_scene) return nullptr;

    auto findResult = m_components.find(pComponentType);
    if (findResult != m_components.end()) { return findResult->second; }

    return nullptr;
}

std::vector<MComponent*> MEntity::GetComponents()
{
    std::vector<MComponent*> vResult;

    for (auto& pr: m_components) { vResult.push_back(pr.second); }

    return vResult;
}

MEngine* MEntity::GetEngine()
{
    if (m_scene) { return m_scene->GetEngine(); }

    return nullptr;
}

void MEntity::DeleteSelf()
{
    if (m_scene) { m_scene->DeleteEntity(this); }
}

flatbuffers::Offset<void> MEntity::Serialize(flatbuffers::FlatBufferBuilder& builder)
{
    MGuid                                               id = GetID();
    fbs::MGuid                                          fbguid(id.data[0], id.data[1], id.data[2], id.data[3]);
    auto                                                fbname = builder.CreateString(GetName());

    std::vector<flatbuffers::Offset<fbs::AnyComponent>> vFbComponents;
    std::vector<MComponent*>&&                          vComponents = GetComponents();
    for (MComponent* pComponent: vComponents)
    {
        flatbuffers::FlatBufferBuilder compBuilder;
        flatbuffers::Offset<void>&&    componentRoot = pComponent->Serialize(compBuilder);
        compBuilder.Finish(componentRoot);

        flatbuffers::Offset<flatbuffers::Vector<uint8_t>>&& fbdata =
                builder.CreateVector(compBuilder.GetBufferPointer(), compBuilder.GetSize());
        flatbuffers::Offset<fbs::AnyComponent>&& fbcomponent =
                fbs::CreateAnyComponent(builder, builder.CreateString(pComponent->GetTypeName().c_str()), fbdata);

        vFbComponents.push_back(fbcomponent);
    }

    auto                fb_components = builder.CreateVector(vFbComponents);

    fbs::MEntityBuilder entityBuilder(builder);

    entityBuilder.add_id(&fbguid);
    entityBuilder.add_name(fbname);
    entityBuilder.add_components(fb_components);

    return entityBuilder.Finish().Union();
}

void MEntity::Deserialize(const void* pBufferPointer)
{
    const fbs::MEntity* fbEntity = reinterpret_cast<const fbs::MEntity*>(pBufferPointer);

    //const fbs::MGuid * fbguid = fbEntity->id();
    //m_id = MGuid(fbguid->data0(), fbguid->data1(), fbguid->data2(), fbguid->data3());
    m_strName = fbEntity->name()->c_str();

    const flatbuffers::Vector<flatbuffers::Offset<fbs::AnyComponent>>& vfbcomponents = *fbEntity->components();

    for (size_t i = 0; i < vfbcomponents.size(); ++i)
    {
        auto                           fbcomponent = vfbcomponents.Get(static_cast<flatbuffers::uoffset_t>(i));

        MStringId                      type = MStringId(fbcomponent->type()->c_str());

        const MType*                   pType      = MTypeClass::GetType(type);
        MComponent*                    pComponent = GetScene()->AddComponent(this, pType);

        flatbuffers::FlatBufferBuilder fbb;
        fbb.PushBytes(fbcomponent->data()->data(), fbcomponent->data()->size());
        pComponent->Deserialize(fbb);
    }
}

void MEntity::PostDeserialize(const std::map<MGuid, MGuid>& tRedirectGuid)
{
    for (auto& pr: m_components)
    {
        if (MComponent* pComponent = pr.second) { pComponent->PostDeserialize(tRedirectGuid); }
    }
}
