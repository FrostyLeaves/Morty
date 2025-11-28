/**
 * @File         MIDPool
 *
 * @Created      2019-08-06 18:39:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"
#include <queue>

namespace morty
{

// Simple incrementing ID pool - IDs are never reused
template<typename TID> class MORTY_API MIDPool
{
public:
    MIDPool()
        : m_nextID(0)
    {}

    TID AllocateID() { return m_nextID++; }

private:
    TID m_nextID;
};

// Reusable ID pool - freed IDs can be reused
template<typename TID> class MORTY_API MReusableIDPool
{
public:
    MReusableIDPool()
        : m_nextID(0)
    {}

    TID AllocateID()
    {
        if (m_freeIDs.empty()) { return m_nextID++; }
        else
        {
            TID id = m_freeIDs.front();
            m_freeIDs.pop();
            return id;
        }
    }

    void FreeID(const TID& id) { m_freeIDs.push(id); }


private:
    TID             m_nextID;
    std::queue<TID> m_freeIDs;
};


}// namespace morty
