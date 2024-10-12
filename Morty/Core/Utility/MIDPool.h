/**
 * @File         MIDPool
 * 
 * @Created      2019-08-06 18:39:21
 *
 * @Author       DoubleYe
**/

#pragma once

#include "Utility/MGlobal.h"

namespace morty
{

template<typename IDTYPE> class MORTY_API MIDPool
{
public:
    MIDPool()
        : m_IDPool(0)
    {}

    IDTYPE GetNewID() { return m_IDPool++; }

private:
    IDTYPE m_IDPool;
};

template<typename IDTYPE> class MORTY_API MRepeatIDPool
{
public:
    MRepeatIDPool()
        : m_IDPool(0)
    {}

    IDTYPE GetNewID()
    {
        if (m_iDPool.empty()) return m_IDPool++;
        else
        {
            IDTYPE id = m_iDPool.front();
            m_iDPool.pop();
            return id;
        }
    }

    void RecoveryID(const IDTYPE& id) { m_iDPool.push(id); }

private:
    IDTYPE             m_IDPool;
    std::queue<IDTYPE> m_iDPool;
};

}// namespace morty