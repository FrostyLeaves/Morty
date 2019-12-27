/**
 * @File         MRenderOrganizer
 * 
 * @Created      2019-09-19 00:23:50
 *
 * @Author       Pobrecito
**/

#ifndef _M_MRENDERORGANIZER_H_
#define _M_MRENDERORGANIZER_H_
#include "MGlobal.h"

class MIRenderer;
class MORTY_CLASS MRenderOrganizer
{
public:
    MRenderOrganizer();
    virtual ~MRenderOrganizer();

public:

	virtual bool Initialize() ;
	virtual void Release() ;

private:

	MIRenderer* m_pRenderer;

};


#endif
