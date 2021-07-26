/**
 * @File         MComponent
 * 
 * @Created      2021-04-26 16:26:56
 *
 * @Author       Pobrecito
**/

#ifndef _M_MCOMPONENT_H_
#define _M_MCOMPONENT_H_
#include "MGlobal.h"
#include "MType.h"

#include "MSerializer.h"

class MEntity;
class MScene;
class MEngine;

class MORTY_API MComponentID
{
public:
    MComponentID() :pComponentType(nullptr), nID(0) {}
    MComponentID(const MType* type, const size_t& id) : pComponentType(type), nID(id) {}

    bool IsValid();

    bool operator ==(const MComponentID& id) const;
    bool operator ==(const MType* pType) const;

	bool operator < (const MComponentID& id) const;
	bool operator < (const MType* pType) const;

public:
    const MType* pComponentType;
    size_t nID;
};

class MORTY_API MComponent : public MTypeClass, public MSerializer
{
public:
    MORTY_CLASS(MComponent)

public:
    MComponent();
    virtual ~MComponent();

public:

    void Initialize(MScene* pScene, const MEntityID& id);

    bool IsValid() { return m_bValid; }

    virtual void Initialize();
    virtual void Release();


public:

    virtual void WriteToStruct(MStruct& srt) override {}

    virtual void ReadFromStruct(const MStruct& srt) override {}

public:

    void SendComponentNotify(const std::string& notify) {}

	MScene* GetScene() { return m_pScene; }
	MEntity* GetEntity();
    MEngine* GetEngine();
    const MComponentID& GetComponentID() { return m_id; }

private:
//property

	MComponentID m_id;

private:
	MEntityID m_entityID;
    MScene* m_pScene;

    bool m_bValid;
};

#endif
