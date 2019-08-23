/**
 * @File         MMeshInstance
 * 
 * @Created      2019-05-26 16:13:55
 *
 * @Author       Morty
**/

#ifndef _M_MMESHINSTANCE_H_
#define _M_MMESHINSTANCE_H_
#include "MGlobal.h"

#include "M3DNode.h"

class MResource;
class MModelResource;
class MORTY_CLASS MMeshInstance : public M3DNode
{
public:
    MMeshInstance();
    virtual ~MMeshInstance();

public:

	bool Load(MResource* pResource);


	virtual void OnTick(const float& fDelta);
	virtual void Render();

private:

	MModelResource* m_pResource;

};


#endif
