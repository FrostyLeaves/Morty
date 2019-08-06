/**
 * @File         MResource
 * 
 * @Created      2019-07-31 19:52:11
 *
 * @Author       Morty
**/

#ifndef _M_MRESOURCE_H_
#define _M_MRESOURCE_H_
#include "MGlobal.h"
#include "MSingleInstance.h"
#include "MIDPool.h"

#include <map>

class MORTY_CLASS MResource
{
public:
    MResource();
    virtual ~MResource();

public:

private:
    
    friend class MResourceManager;
    
    MResourceID m_unResourceID;

};

class MResourceManager : public MSingleInstance<MResourceManager>
{
public:
    MResourceManager();
    virtual ~MResourceManager();
    
    template<typename Resource_TYPE>
    Resource_TYPE* CreateResource()
    {
        Resource_TYPE* pResource = new Resource_TYPE();
        if (!dynamic_cast<MResource*>(pResource))
        {
            delete pResource;
            return nullptr;
        }
        
        pResource->m_unResourceID = m_pResourceDB->GetNewID();
        
        m_tResources[pResource->m_unResourceID] = pResource;
        
        return pResource;
    }
    
private:
    
    std::map<MResourceID, MResource*> m_tResources;
    
    MIDPool<MResourceID>* m_pResourceDB;
};


#endif
