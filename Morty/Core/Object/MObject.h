/**
 * @File         MObject
 * 
 * @Created      2019-05-25 19:43:33
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

#include "Type/MType.h"
#include "Utility/MString.h"

namespace morty
{

class MEngine;
class MAutoPtr;
class MObjectSystem;
class MORTY_API MObject : public MTypeClass
{
public:
                            MORTY_CLASS(MObject);

                            MObject()          = default;
    ~                       MObject() override = default;
    [[nodiscard]] MObjectID GetObjectID() const { return m_objectID; }
    [[nodiscard]] MEngine*  GetEngine() const { return m_engine; }
    MObjectSystem*          GetObjectSystem();
    void                    DeleteLater();
    [[nodiscard]] bool      GetDeleteMark() const { return m_deleteMark; }
    virtual void            OnCreated(){};
    virtual void            OnDelete(){};

protected:
    friend class MObjectSystem;

    MObjectID m_objectID   = 0;
    MEngine*  m_engine     = nullptr;
    bool      m_deleteMark = false;
};

}// namespace morty