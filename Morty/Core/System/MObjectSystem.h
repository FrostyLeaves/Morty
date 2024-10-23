/**
 * @File         MObject
 * 
 * @Created      2019-05-25 19:43:33
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

#include "Engine/MSystem.h"
#include "Object/MObject.h"
#include "Type/MType.h"
#include "Utility/MIDPool.h"
#include "Utility/MString.h"

namespace morty
{

class MEngine;
class MORTY_API MObjectSystem : public MISystem
{
public:
    MORTY_CLASS(MObjectSystem)
public:
    MObjectSystem();

    virtual ~MObjectSystem();

    void                                        InitObject(MObject* pObject);

    template<typename OBJECT_TYPE> OBJECT_TYPE* CreateObject()
    {
        OBJECT_TYPE* pObject = new OBJECT_TYPE();
        if (!dynamic_cast<MObject*>(pObject))
        {
            delete pObject;
            return nullptr;
        }

        InitObject(pObject);
        return pObject;
    }

    MObject* CreateObject(const MStringId& strTypeName);

    MObject* FindObject(const MObjectID& unID);

    void     RemoveObject(const MObjectID& unID);

    void     CleanRemoveObject();

public:
    typedef std::function<void(MObject*)> PostCreateObjectFunction;

    void                                  RegisterPostCreateObject(const PostCreateObjectFunction& func);

public:
    virtual void Release() override;

private:
    MIDPool<MObjectID>*                   m_objectDB;

    std::map<MObjectID, MObject*>         m_objects;

    std::vector<MObjectID>                m_removeObjects;

    std::vector<PostCreateObjectFunction> m_postCreateObjectFunction;
};

}// namespace morty