/**
 * @File         MComponent
 * 
 * @Created      2021-04-26 16:26:56
 *
 * @Author       DoubleYe
**/

#pragma once
#include "Utility/MGlobal.h"
#include "Type/MType.h"

#include "Scene/MGuid.h"
#include "flatbuffers/flatbuffer_builder.h"

MORTY_SPACE_BEGIN

class MScene;
class MEntity;
class MEngine;
class MComponent;

class MORTY_API MComponentID
{
public:
    MComponentID() : pComponentType(nullptr), nIdx(0) {}
    MComponentID(const MType* type, size_t id) : pComponentType(type), nIdx(id) {}

    bool IsValid() const;

    bool operator ==(const MComponentID& id) const;
    bool operator ==(const MType* pType) const;

	bool operator < (const MComponentID& id) const;
	bool operator < (const MType* pType) const;

public:
    const MType* pComponentType;
	size_t nIdx;
};

class MORTY_API MComponent : public MTypeClass
{
public:
    MORTY_CLASS(MComponent)

public:
    MComponent();
    virtual ~MComponent();

public:

    void Initialize(MScene* pScene, const MGuid& id);
    virtual void Release();

    bool IsValid() const { return m_bValid; }


public:

    virtual flatbuffers::Offset<void> Serialize(flatbuffers::FlatBufferBuilder& fbb);
    virtual void Deserialize(flatbuffers::FlatBufferBuilder& fbb);
	virtual void Deserialize(const void* pBufferPointer);
	virtual void PostDeserialize(const std::map<MGuid, MGuid>& tRedirectGuid);

public:

    void SendComponentNotify(const char* notify);

	MScene* GetScene() const { return m_pScene; }
	MEntity* GetEntity() const;
    MEngine* GetEngine() const;

    void SetComponentID(const MComponentID& id) { m_id = id; }
    const MComponentID& GetComponentID() { return m_id; }

private:
//property
	MComponentID m_id;
	MGuid m_entityID;
    MScene* m_pScene;

    bool m_bValid;
};

MORTY_SPACE_END