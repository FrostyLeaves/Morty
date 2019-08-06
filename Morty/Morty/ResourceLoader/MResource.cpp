#include "MResource.h"

MResource::MResource()
: m_unResourceID(0)
{
    
}

MResource::~MResource()
{

}
        
MResourceManager::MResourceManager()
: m_pResourceDB(new MIDPool<MResourceID>())
{
    
}

MResourceManager::~MResourceManager()
{
    
}
