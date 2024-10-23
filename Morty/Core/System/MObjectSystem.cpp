#include "System/MObjectSystem.h"
#include "Engine/MEngine.h"
#include "MObjectSystem.h"

using namespace morty;

MORTY_CLASS_IMPLEMENT(MObjectSystem, MISystem)

MObjectSystem::MObjectSystem()
    : m_objectDB(new MIDPool<MObjectID>())
{}

MObjectSystem::~MObjectSystem() {}

void MObjectSystem::InitObject(MObject* pObject)
{
    pObject->m_unObjectID = m_objectDB->GetNewID();
    pObject->m_engine     = GetEngine();

    m_objects[pObject->m_unObjectID] = pObject;

    pObject->OnCreated();


    for (auto func: m_postCreateObjectFunction) { func(pObject); }
}

MObject* MObjectSystem::CreateObject(const MStringId& strTypeName)
{
    if (MTypeClass* pTypedIns = MTypeClass::New(strTypeName))
    {
        if (MObject* pObject = pTypedIns->template DynamicCast<MObject>())
        {
            InitObject(pObject);
            return pObject;
        }
    }

    return nullptr;
}

MObject* MObjectSystem::FindObject(const MObjectID& unID)
{
    std::map<MObjectID, MObject*>::iterator iter = m_objects.find(unID);
    if (iter == m_objects.end()) return nullptr;

    return iter->second;
}

void MObjectSystem::RemoveObject(const MObjectID& unID) { m_removeObjects.push_back(unID); }

void MObjectSystem::CleanRemoveObject()
{
    while (!m_removeObjects.empty())
    {
        const MObjectID& unID = m_removeObjects.back();
        m_removeObjects.pop_back();

        std::map<MObjectID, MObject*>::iterator iter = m_objects.find(unID);
        if (iter != m_objects.end())
        {
            iter->second->OnDelete();
            delete iter->second;
            m_objects.erase(iter);
        }
    }
}

void MObjectSystem::RegisterPostCreateObject(const PostCreateObjectFunction& func)
{
    m_postCreateObjectFunction.push_back(func);
}

void MObjectSystem::Release()
{
    Super::Release();

    while (!m_objects.empty() || !m_removeObjects.empty())
    {
        std::map<MObjectID, MObject*> tObjects = std::move(m_objects);
        m_objects                              = {};

        for (std::map<MObjectID, MObject*>::iterator iter = tObjects.begin(); iter != tObjects.end(); ++iter)
        {
            iter->second->OnDelete();
            delete iter->second;
            iter->second = nullptr;
        }

        CleanRemoveObject();
    }

    delete m_objectDB;
    m_objectDB = nullptr;
}
