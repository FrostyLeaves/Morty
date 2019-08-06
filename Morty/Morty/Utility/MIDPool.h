/**
 * @File         MIDPool
 * 
 * @Created      2019-08-06 18:39:21
 *
 * @Author       Morty
**/

#ifndef _M_MIDPOOL_H_
#define _M_MIDPOOL_H_
#include "MGlobal.h"

template<typename IDTYPE>
class MORTY_CLASS MIDPool
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


#endif
