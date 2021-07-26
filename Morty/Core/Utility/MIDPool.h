/**
 * @File         MIDPool
 * 
 * @Created      2019-08-06 18:39:21
 *
 * @Author       DoubleYe
**/

#ifndef _M_MIDPOOL_H_
#define _M_MIDPOOL_H_
#include "MGlobal.h"
#include <queue>

template<typename IDTYPE>
class MORTY_API MIDPool
{
public:
    MIDPool():m_IDPool(0){}
    
    IDTYPE GetNewID()
    {
        return ++m_IDPool;
    }
    
private:
    IDTYPE m_IDPool;
};

template<typename IDTYPE>
class MORTY_API MRepeatIDPool
{
public:
    MRepeatIDPool() :m_IDPool(0) {}

	IDTYPE GetNewID()
	{
        if(m_vIDPool.empty())
		    return ++m_IDPool;
        else
        {
            IDTYPE id = m_vIDPool.front();
            m_vIDPool.pop();
            return id;
        }
	}

    void RecoveryID(const IDTYPE& id)
    {
        m_vIDPool.push(id);
    }

private:
	IDTYPE m_IDPool;
    std::queue<IDTYPE> m_vIDPool;
};


#endif
